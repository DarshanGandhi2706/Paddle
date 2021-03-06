//   Copyright (c) 2018 PaddlePaddle Authors. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
#pragma once
#include <limits>
#include <vector>
#include "paddle/fluid/operators/math/math_function.h"

namespace paddle {
namespace operators {
namespace math {

template <typename T>
struct CBlas;

#ifdef PADDLE_WITH_MKLML
template <>
struct CBlas<float> {
  template <typename... ARGS>
  static void GEMM(ARGS... args) {
    platform::dynload::cblas_sgemm(args...);
  }

  template <typename... ARGS>
  static float *GEMM_ALLOC(ARGS... args) {
    return platform::dynload::cblas_sgemm_alloc(args...);
  }

  template <typename... ARGS>
  static void GEMM_PACK(ARGS... args) {
    platform::dynload::cblas_sgemm_pack(args...);
  }

  template <typename... ARGS>
  static void GEMM_COMPUTE(ARGS... args) {
    platform::dynload::cblas_sgemm_compute(args...);
  }

  template <typename... ARGS>
  static void GEMM_FREE(ARGS... args) {
    platform::dynload::cblas_sgemm_free(args...);
  }

#ifdef PADDLE_WITH_LIBXSMM
  template <typename... ARGS>
  static void SMM_GEMM(ARGS... args) {
    libxsmm_sgemm(args...);
  }
#endif

  template <typename... ARGS>
  static void AXPY(ARGS... args) {
    platform::dynload::cblas_saxpy(args...);
  }

  template <typename... ARGS>
  static void VCOPY(ARGS... args) {
    platform::dynload::cblas_scopy(args...);
  }

  template <typename... ARGS>
  static void GEMV(ARGS... args) {
    platform::dynload::cblas_sgemv(args...);
  }

  template <typename... ARGS>
  static void GEMM_BATCH(ARGS... args) {
    platform::dynload::cblas_sgemm_batch(args...);
  }

  template <typename... ARGS>
  static void VADD(ARGS... args) {
    platform::dynload::vsAdd(args...);
  }
};

template <>
struct CBlas<double> {
  template <typename... ARGS>
  static void GEMM(ARGS... args) {
    platform::dynload::cblas_dgemm(args...);
  }

  template <typename... ARGS>
  static double *GEMM_ALLOC(ARGS... args) {
    return platform::dynload::cblas_dgemm_alloc(args...);
  }

  template <typename... ARGS>
  static void GEMM_PACK(ARGS... args) {
    platform::dynload::cblas_dgemm_pack(args...);
  }

  template <typename... ARGS>
  static void GEMM_COMPUTE(ARGS... args) {
    platform::dynload::cblas_dgemm_compute(args...);
  }

  template <typename... ARGS>
  static void GEMM_FREE(ARGS... args) {
    platform::dynload::cblas_dgemm_free(args...);
  }

#ifdef PADDLE_WITH_LIBXSMM
  template <typename... ARGS>
  static void SMM_GEMM(ARGS... args) {
    libxsmm_dgemm(args...);
  }
#endif

  template <typename... ARGS>
  static void AXPY(ARGS... args) {
    platform::dynload::cblas_daxpy(args...);
  }

  template <typename... ARGS>
  static void VCOPY(ARGS... args) {
    platform::dynload::cblas_dcopy(args...);
  }

  template <typename... ARGS>
  static void GEMV(ARGS... args) {
    platform::dynload::cblas_dgemv(args...);
  }

  template <typename... ARGS>
  static void GEMM_BATCH(ARGS... args) {
    platform::dynload::cblas_dgemm_batch(args...);
  }

  template <typename... ARGS>
  static void VADD(ARGS... args) {
    platform::dynload::vdAdd(args...);
  }
};

#else

template <>
struct CBlas<float> {
  template <typename... ARGS>
  static void GEMM(ARGS... args) {
    cblas_sgemm(args...);
  }

  template <typename... ARGS>
  static void AXPY(ARGS... args) {
    cblas_saxpy(args...);
  }

  template <typename... ARGS>
  static void VCOPY(ARGS... args) {
    cblas_scopy(args...);
  }

  template <typename... ARGS>
  static void GEMV(ARGS... args) {
    cblas_sgemv(args...);
  }
};

template <>
struct CBlas<double> {
  template <typename... ARGS>
  static void GEMM(ARGS... args) {
    cblas_dgemm(args...);
  }

  template <typename... ARGS>
  static void AXPY(ARGS... args) {
    cblas_daxpy(args...);
  }

  template <typename... ARGS>
  static void VCOPY(ARGS... args) {
    cblas_dcopy(args...);
  }

  template <typename... ARGS>
  static void GEMV(ARGS... args) {
    cblas_dgemv(args...);
  }
};
#endif

template <>
struct CBlas<platform::float16> {
  static void GEMM(...) { PADDLE_THROW("float16 GEMM not supported on CPU"); }
  static void SMM_GEMM(...) {
    PADDLE_THROW("float16 SMM_GEMM not supported on CPU");
  }
#ifdef PADDLE_WITH_MKLML
  static void GEMM_BATCH(...) {
    PADDLE_THROW("float16 GEMM_BATCH not supported on CPU");
  }
#endif
};

template <typename T>
inline bool UseXSMM(const int &m, const int &n, const int &k, bool transa,
                    bool transb, const T &alpha, const T &beta) {
#ifdef PADDLE_WITH_LIBXSMM
  // Refer to https://github.com/hfp/libxsmm/blob/master/README.md
  // But the threshold is custom
  constexpr int LIBXSMM_THRESHOLD = 20 * 20 * 20;
  if (m * n * k > LIBXSMM_THRESHOLD || transa || transb ||
      std::abs<T>(alpha - static_cast<T>(1) >
                  std::numeric_limits<T>::epsilon()) ||
      std::abs<T>(beta) > std::numeric_limits<T>::epsilon()) {
    return false;
  } else {
    return true;
  }
#endif
  return false;
}

template <>
inline bool UseXSMM<platform::float16>(const int &m, const int &n, const int &k,
                                       bool transa, bool transb,
                                       const platform::float16 &alpha,
                                       const platform::float16 &beta) {
  return false;
}

template <typename T>
inline void GEMM_WARP(CBLAS_ORDER order, CBLAS_TRANSPOSE transA,
                      CBLAS_TRANSPOSE transB, int M, int N, int K, T alpha,
                      const T *A, int lda, const T *B, int ldb, T beta, T *C,
                      int ldc) {
#ifdef PADDLE_WITH_LIBXSMM
  if (UseXSMM<T>(M, N, K, transA != CblasNoTrans, transB != CblasNoTrans, alpha,
                 beta)) {
    // Note: SMM use ColMajor
    const char transa = 'N';
    const char transb = 'N';
    CBlas<T>::SMM_GEMM(&transa, &transb, &N, &M, &K, &alpha, B, &ldb, A, &lda,
                       &beta, C, &ldc);
    return;
  }
#endif

#ifdef PADDLE_MKL_SPLIT_GEMM
  constexpr int bs = 2;
  if (M % bs == 0 && transA == CblasNoTrans && transB == CblasNoTrans) {
    for (int off = 0; off < M; off += bs) {
      CBlas<T>::GEMM(CblasRowMajor, CblasNoTrans, CblasNoTrans, bs, N, K, alpha,
                     A + off * lda, lda, B, ldb, beta, C + off * ldb, ldc);
    }
    return;
  }
#endif
  CBlas<T>::GEMM(CblasRowMajor, transA, transB, M, N, K, alpha, A, lda, B, ldb,
                 beta, C, ldc);
}

#ifdef PADDLE_WITH_MKLML
template <>
template <typename T>
T *Blas<platform::CPUDeviceContext>::GEMM_ALLOC(const CBLAS_IDENTIFIER id,
                                                const int M, const int N,
                                                const int K) const {
  return CBlas<T>::GEMM_ALLOC(id, M, N, K);
}

template <>
template <typename T>
void Blas<platform::CPUDeviceContext>::GEMM_PACK(const CBLAS_IDENTIFIER id,
                                                 const CBLAS_TRANSPOSE trans,
                                                 int M, int N, int K,
                                                 const T alpha, const T *src,
                                                 const int ld, T *dst) const {
  CBlas<T>::GEMM_PACK(CblasRowMajor, id, trans, M, N, K, alpha, src, ld, dst);
}

template <>
template <typename T>
void Blas<platform::CPUDeviceContext>::GEMM_COMPUTE(
    int transA, int transB, int M, int N, int K, const T *A, const int lda,
    const T *B, const int ldb, T beta, T *C, const int ldc) const {
  CBlas<T>::GEMM_COMPUTE(CblasRowMajor, transA, transB, M, N, K, A, lda, B, ldb,
                         beta, C, ldc);
}

template <>
template <typename T>
void Blas<platform::CPUDeviceContext>::GEMM_FREE(T *data) const {
  CBlas<T>::GEMM_FREE(data);
}
#endif

template <>
template <typename T>
void Blas<platform::CPUDeviceContext>::GEMM(CBLAS_TRANSPOSE transA,
                                            CBLAS_TRANSPOSE transB, int M,
                                            int N, int K, T alpha, const T *A,
                                            const T *B, T beta, T *C) const {
  int lda = (transA == CblasNoTrans) ? K : M;
  int ldb = (transB == CblasNoTrans) ? N : K;
  int ldc = N;
  GEMM_WARP<T>(CblasRowMajor, transA, transB, M, N, K, alpha, A, lda, B, ldb,
               beta, C, ldc);
}

template <>
template <typename T>
void Blas<platform::CPUDeviceContext>::GEMM(bool transA, bool transB, int M,
                                            int N, int K, T alpha, const T *A,
                                            int lda, const T *B, int ldb,
                                            T beta, T *C, int ldc) const {
  GEMM_WARP<T>(CblasRowMajor, transA == false ? CblasNoTrans : CblasTrans,
               transB == false ? CblasNoTrans : CblasTrans, M, N, K, alpha, A,
               lda, B, ldb, beta, C, ldc);
}

template <typename DeviceContext>
template <typename T>
void Blas<DeviceContext>::MatMul(const framework::Tensor &mat_a, bool trans_a,
                                 const framework::Tensor &mat_b, bool trans_b,
                                 T alpha, framework::Tensor *mat_out,
                                 T beta) const {
  auto dim_a = mat_a.dims();
  auto dim_b = mat_b.dims();
  auto dim_out = mat_out->dims();
  PADDLE_ENFORCE(dim_a.size() == 2 && dim_b.size() == 2 && dim_out.size() == 2,
                 "The input and output of matmul be matrix");
  PADDLE_ENFORCE(
      mat_a.place() == mat_b.place() && mat_a.place() == mat_out->place(),
      "The places of matrices must be same");

  int M = dim_out[0];
  int N = dim_out[1];
  int K = !trans_a ? dim_a[1] : dim_a[0];

  CBLAS_TRANSPOSE transA = !trans_a ? CblasNoTrans : CblasTrans;
  CBLAS_TRANSPOSE transB = !trans_b ? CblasNoTrans : CblasTrans;

  this->GEMM(transA, transB, M, N, K, alpha, mat_a.data<T>(), mat_b.data<T>(),
             beta, mat_out->data<T>());
}

template <>
template <typename T>
void Blas<platform::CPUDeviceContext>::AXPY(int n, T alpha, const T *x,
                                            T *y) const {
  CBlas<T>::AXPY(n, alpha, x, 1, y, 1);
}

template <>
template <typename T>
void Blas<platform::CPUDeviceContext>::VCOPY(int n, const T *x, T *y) const {
  CBlas<T>::VCOPY(n, x, 1, y, 1);
}

template <>
template <typename T>
void Blas<platform::CPUDeviceContext>::VADD(int n, const T *x, const T *y,
                                            T *z) const {
#ifdef PADDLE_WITH_MKLML
  CBlas<T>::VADD(n, x, y, z);
#else
  this->template VCOPY<T>(n, y, z);
  this->template AXPY<T>(n, 1., x, z);
#endif
}

template <>
template <typename T>
void Blas<platform::CPUDeviceContext>::GEMV(bool trans_a, int M, int N, T alpha,
                                            const T *A, const T *B, T beta,
                                            T *C) const {
  CBLAS_TRANSPOSE transA = !trans_a ? CblasNoTrans : CblasTrans;
  CBlas<T>::GEMV(CblasRowMajor, transA, M, N, alpha, A, N, B, 1, beta, C, 1);
}

template <>
template <typename T>
void Blas<platform::CPUDeviceContext>::BatchedGEMM(
    CBLAS_TRANSPOSE transA, CBLAS_TRANSPOSE transB, int M, int N, int K,
    T alpha, const T *A, const T *B, T beta, T *C, int batchCount,
    int64_t strideA, int64_t strideB) const {
#ifdef PADDLE_WITH_MKLML
  int lda = (transA == CblasNoTrans) ? K : M;
  int ldb = (transB == CblasNoTrans) ? N : K;
  int ldc = N;
  auto a_array = std::vector<const T *>(batchCount);
  auto b_array = std::vector<const T *>(batchCount);
  auto c_array = std::vector<T *>(batchCount);
  for (int k = 0; k < batchCount; ++k) {
    a_array[k] = &A[k * strideA];
    b_array[k] = &B[k * strideB];
    c_array[k] = &C[k * M * N];
  }

  CBlas<T>::GEMM_BATCH(CblasRowMajor, &transA, &transB, &M, &N, &K, &alpha,
                       a_array.data(), &lda, b_array.data(), &ldb, &beta,
                       c_array.data(), &ldc, 1 /* group_count */, &batchCount);
#else
  for (int k = 0; k < batchCount; ++k) {
    auto *Ak = &A[k * strideA];
    auto *Bk = &B[k * strideB];
    auto *Ck = &C[k * M * N];
    this->template GEMM<T>(transA, transB, M, N, K, alpha, Ak, Bk, beta, Ck);
  }
#endif
}

template <typename DeviceContext>
template <typename T>
void Blas<DeviceContext>::MatMul(const framework::Tensor &mat_a,
                                 const MatDescriptor &dim_a,
                                 const framework::Tensor &mat_b,
                                 const MatDescriptor &dim_b, T alpha,
                                 framework::Tensor *mat_out, T beta) const {
  PADDLE_ENFORCE_EQ(dim_a.width_, dim_b.height_);
  CBLAS_TRANSPOSE transA = !dim_a.trans_ ? CblasNoTrans : CblasTrans;
  CBLAS_TRANSPOSE transB = !dim_b.trans_ ? CblasNoTrans : CblasTrans;
  if (dim_a.batch_size_ == 0 && dim_b.batch_size_ == 0) {
    this->template GEMM<T>(transA, transB, dim_a.height_, dim_b.width_,
                           dim_a.width_, alpha, mat_a.data<T>(),
                           mat_b.data<T>(), beta, mat_out->data<T>());
  } else {
    PADDLE_ENFORCE(dim_a.batch_size_ == dim_b.batch_size_ ||
                   dim_a.batch_size_ == 0 || dim_b.batch_size_ == 0);
    this->template BatchedGEMM<T>(
        transA, transB, dim_a.height_, dim_b.width_, dim_a.width_, alpha,
        mat_a.data<T>(), mat_b.data<T>(), beta, mat_out->data<T>(),
        dim_a.batch_size_ == 0 ? dim_b.batch_size_ : dim_a.batch_size_,
        dim_a.stride_, dim_b.stride_);
  }
}

}  // namespace math
}  // namespace operators
}  // namespace paddle
