#ifndef SYNTH_MUL
#define SYNTH_MUL

#include "tao.h"
#include "dtypes.h"

#include <chrono>
#include <iostream>
#include <atomic>
#include <cmath>

#define PSLACK 8  

// Matrix multiplication, tao groupation on written value
class Synth_MatMul : public AssemblyTask 
{
public: 
// initialize static parameters
#if defined(CRIT_PERF_SCHED)
  static float time_table[][GOTAO_NTHREADS];
#endif

  Synth_MatMul(uint32_t _size, int _width): AssemblyTask(_width) {   
    dim_size = _size;
    block_size = dim_size / (_width * PSLACK);
    if(block_size == 0) block_size = 1;
    block_index = 0;
    uint32_t elem_count = dim_size * dim_size;
    A = new real_t[elem_count]; 
    B_Trans = new real_t[elem_count];
    C = new real_t[elem_count];
    block_count = dim_size / block_size;
  }

  int cleanup() { 
    delete[] A;
    delete[] B_Trans;
    delete[] C;
  }

  // this assembly can work totally asynchronously
  int execute(int threadid) {
    while(true) {
      int row_block_id = block_index++;
      if(row_block_id > block_count) return 0;
      // assume B is transposed, so that you can utilize the performance of transposed matmul 
      for (int i = row_block_id * block_size; i < dim_size && i < ((row_block_id + 1 ) * block_size); ++i) { 
        for (int j = 0; j < dim_size; j++) {
          real_t res  = 0;
          for (int k = 0; k < dim_size; k++) {
            res += A[i*dim_size+k]*B_Trans[j*dim_size+k];
          }
          C[i*dim_size+j] = res;
        }
      }
    }
  }

#if defined(CRIT_PERF_SCHED)
  int set_timetable(int threadid, float ticks, int index) {
    time_table[index][threadid] = ticks;
  }

  float get_timetable(int threadid, int index) { 
    float time=0;
    time = time_table[index][threadid];
    return time;
  }
#endif
private:
  std::atomic<int> block_index; 
  int dim_size;
  int block_count;
  int block_size;
  real_t* A, *B_Trans, *C;
};

#endif