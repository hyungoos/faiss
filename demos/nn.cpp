#include <iostream>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cassert>
#include <cstring>
#include <fstream>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <sys/time.h>

#include "../AutoTune.h"
#include "../args.h"
#include "../index_io.h"
#include "../IndexHNSW.h"



using namespace faiss;

double elapsed ()
{
    struct timeval tv;
    gettimeofday (&tv, nullptr);
    return  tv.tv_sec + tv.tv_usec * 1e-6;
}

float * vecs_read (std::string fname, size_t *dim, size_t *n, std::vector<std::string> *words) {
    std::ifstream in(fname);
    if (!in.is_open()) {
        throw std::invalid_argument(fname + " cannot be opened for loading!");
    }
    in >> *n >> *dim;
    std::cout << *n << std::endl;
    std::cout << *dim << std::endl;
    float *x = new float[*n * *dim];
    for (size_t i = 0; i < *n; i++) {
        if (i % 1000000 == 0) {
            std::cerr << "\rRead " << i  / 1000000 << "M vecs" << std::flush;
        }
        std::string word;
        in >> word;
        words->push_back(word);
        for (size_t j = 0; j < *dim; j++) {
            in >> x[*dim * i + j];
        }
    }
    in.close();
    assert((words->size() == *n) || !"word vectors size mismatch");
    return x;
}

std::ifstream initialize_vecs_read_batch (std::string fname, size_t *dim, size_t *n) {
    std::ifstream in(fname);
    if (!in.is_open()) {
        throw std::invalid_argument(fname + " cannot be opened for loading!");
    }
    in >> *n >> *dim;
    std::cout << *n << std::endl;
    std::cout << *dim << std::endl;
    return in;
}

void next_batch (std::ifstream& in, int batchSize, size_t *dim, float *x) {
    for (size_t i = 0; i < batchSize; i++) {
        std::string word;
        in >> word; // word is discarded
        for (size_t j = 0; j < *dim; j++) {
            in >> x[*dim * i + j];
        }
    }
}


void printUsage() {
  std::cerr
    << "usage: nn <args>\n\n"
    << std::endl;
}


int main(int argc, char** argv) {
    std::vector<std::string> args(argv, argv + argc);
    if (args.size() < 2) {
      printUsage();
      exit(EXIT_FAILURE);
    }

    Args args_ = Args();
    args_.parseArgs(args);
    args_.dumpArgs();

    double t0 = elapsed();

    // this is typically the fastest one.
    // const char *index_key = "IVF4096,Flat";

    // these ones have better memory usage
    // const char *index_key = "Flat";
    // const char *index_key = "PQ32";
    // const char *index_key = "PCA80,Flat";
    // const char *index_key = "IVF4096,PQ8+16";
    // const char *index_key = "IVF4096,PQ32";
    // const char *index_key = "IMI2x8,PQ32";
    // const char *index_key = "IMI2x8,PQ8+16";
    // const char *index_key = "OPQ16_64,IMI2x8,PQ8+16";
    //
    // https://github.com/facebookresearch/faiss/wiki/Guidelines-to-choose-an-index
    //const char *index_key = "HNSW4";

    size_t d;
    size_t nt;
    std::vector<std::string> words;
    float *xt = vecs_read(args_.input, &d, &nt, &words);

    printf ("[%.3f s] Loading dataset\n", elapsed() - t0);
    double t1 = elapsed();

    // select index
    faiss::Index * index;
    if (args_.indexKey == "hnswcustom") {
        std::cout << "Using hnswcustom" << std::endl;
        faiss::IndexHNSWFlat * idx = new IndexHNSWFlat(d, args_.linksPerVector);
        idx->hnsw.efConstruction = args_.efConstruction;
        idx->hnsw.efSearch = args_.efSearch;
        index = idx;
    } else {
        index = faiss::index_factory(d, args_.indexKey.c_str());
    } 

    index->add(nt, xt);
    faiss::write_index(index, args_.indexPath.c_str());

    delete [] xt;
    printf ("[%.3f s] Indexed\n", elapsed() - t1);
    double t2 = elapsed();

    int k = args_.K;
    // test xq
    size_t dim;
    size_t n; // total query size - this will be vec size
    size_t batchSize = args_.batchSize;
    size_t nq = batchSize;
    std::ifstream queryStream = initialize_vecs_read_batch(args_.query, &dim, &n);
    std::cout << "dim: " << dim << std::endl;
    std::cout << "no of vecs:" << n << std::endl;
    float * xq = new float[batchSize * dim];
    long *I = new long[k * batchSize];
    float *D = new float[k * batchSize];
    size_t processed = 0;
    std::ofstream ofs(args_.output);
    std::ofstream dist_ofs(args_.output + ".distance");

    if (!ofs.is_open()) {
        throw std::invalid_argument(
            "outPath cannot be opened for saving vectors!");
    }
    if (!dist_ofs.is_open()) {
        throw std::invalid_argument(
            "outPath.distance cannot be opened for saving distances!");
    }
    while (processed < n) {
        // search xq
        nq = std::min(batchSize, n - processed);
        next_batch(queryStream, batchSize, &dim, xq);
        // TODO: read from file to get xt
        index->search(nq, xq, k, D, I);

        # print ids and distances
        for(int i = 0; i < nq; i++) {
            for(int j = 0; j < k; j++) {
                ofs << words[I[i * k + j]] << " ";
                dist_ofs << D[i * k + j] << " ";
            }
            ofs << std::endl;
            dist_ofs << std::endl;

        }
        processed += nq;
        if (processed % 1000000 == 0) {
            std::cout << "\rFound " << processed  / 1000000 << "M neighbours" << std::flush;
            printf ("[%.3f s] Neighbour search time\n", elapsed() - t2);
            t2 = elapsed();
        }
    }
    std::cout << "Done finding neighbours." << std::endl;
    printf ("[%.3f s] Total\n", elapsed() - t0);
    return 0;
}


