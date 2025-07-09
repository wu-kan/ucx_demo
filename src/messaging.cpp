#include "messaging.h"
#include <chrono>
#include <cstring>
#include <string>

ucp_request_param_t request_parameters{};

#define CHECK_CUDA(call)                                                       \
  do {                                                                         \
    const cudaError_t error_code = call;                                       \
    if (error_code != cudaSuccess) {                                           \
      std::fprintf(stderr, "CUDA Error:\n");                                   \
      std::fprintf(stderr, " File:       %s\n", __FILE__);                     \
      std::fprintf(stderr, " Line:       %d\n", __LINE__);                     \
      std::fprintf(stderr, " Err code: %d\n", error_code);                     \
      std::fprintf(stderr, " Err str: %s\n", cudaGetErrorString(error_code));  \
      std::exit(1);                                                            \
    }                                                                          \
  } while (0)

ucs_status_t Messaging::on_client_ready() {
  const size_t msg_max_len = 8LL << 30;
  char *msg_host;
  CHECK_CUDA(cudaMallocHost((void **)&msg_host, sizeof(char) * msg_max_len));
  do {
    std::string message = "Hello UCX on CUDA";
    std::memcpy(msg_host, message.c_str(), sizeof(char) * (message.size() + 1));
  } while (0);

  for (size_t msg_len = 4LL << 30; msg_len <= msg_max_len; msg_len <<= 1) {
    for (int iter = 0; iter < 10; ++iter) {
      auto t1 = std::chrono::high_resolution_clock::now();
      auto request =
          ucp_tag_send_nbx(endpoint, msg_host, msg_len, 0, &request_parameters);

      if (auto status = wait_on_request(request); status != UCS_OK) {

        CHECK_CUDA(cudaFreeHost(msg_host));

        ucs_error("Sending message failed");
        return status;
      }

      auto t2 = std::chrono::high_resolution_clock::now();
      auto us = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1)
                    .count();

      std::string logs =
          "Message sent " +
          std::to_string(msg_len / (1024.0 * 1024.0 * 1024.0)) +
          "GB, latency " + std::to_string(us) + "us, bandwidth " +
          std::to_string(msg_len / (1024.0 * 1024.0 * 1024.0 * (us / 1e6))) +
          "GB/s";
      ucs_info(logs.c_str());
    }
  }

  CHECK_CUDA(cudaFreeHost(msg_host));

  return UCS_OK;
}

ucs_status_t Messaging::on_server_ready() {
  const size_t msg_max_len = 8LL << 30;
  char *msg_device = nullptr;
  CHECK_CUDA(cudaMalloc((void **)&msg_device, sizeof(char) * msg_max_len));

  for (size_t msg_len = 4LL << 30; msg_len <= msg_max_len; msg_len <<= 1) {
    for (int iter = 0; iter < 10; ++iter) {
      auto request = ucp_tag_recv_nbx(worker, msg_device, msg_len, 0, 0,
                                      &request_parameters);

      if (auto status = wait_on_request(request); status != UCS_OK) {
        ucs_error("Receiving message failed");
        CHECK_CUDA(cudaFree(msg_device));
        return status;
      }
    }
  }

  char *msg_host = (char *)std::malloc(sizeof(char) * msg_max_len);
  CHECK_CUDA(cudaMemcpy((void *)msg_host, (void *)msg_device,
                        sizeof(char) * msg_max_len, cudaMemcpyDeviceToHost));
  CHECK_CUDA(cudaFree(msg_device));
  ucs_info("Received %s", msg_host);
  std::free(msg_host);
  return UCS_OK;
}