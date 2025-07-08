#include "messaging.h"
#include <string>

ucp_request_param_t request_parameters {};

ucs_status_t Messaging::on_client_ready() {
    std::string message = "Hello UCX";
    auto request = ucp_tag_send_nbx(
            endpoint,
            message.c_str(),
            message.size() + 1,
            0,
            &request_parameters
    );

    if (auto status = wait_on_request(request); status != UCS_OK) {
        ucs_error("Sending message failed");
        return status;
    }

    ucs_info("Message sent");
    return UCS_OK;
}

#if 1
ucs_status_t Messaging::on_server_ready() {
    std::vector<char> message(32);
    auto request = ucp_tag_recv_nbx(
            worker,
            message.data(),
            message.size(),
            0,
            0,
            &request_parameters
    );

    if (auto status = wait_on_request(request); status != UCS_OK) {
        ucs_error("Receiving message failed");
        return status;
    }

    ucs_info("Received %s", message.data());
    return UCS_OK;
}
#else
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

ucs_status_t Messaging::on_server_ready() {
  const int msg_len = 32;
  char *msg_device = nullptr, msg_host[msg_len] = "";
  CHECK_CUDA(cudaMalloc((void **)&msg_device, sizeof(char) * msg_len));
  auto request =
      ucp_tag_recv_nbx(worker, msg_device, msg_len, 0, 0, &request_parameters);

  if (auto status = wait_on_request(request); status != UCS_OK) {
    ucs_error("Receiving message failed");
    return status;
  }

  CHECK_CUDA(cudaMemcpy((void *)msg_host, (void *)msg_device,
                        sizeof(char) * msg_len, cudaMemcpyDeviceToHost));
  CHECK_CUDA(cudaFree(msg_device));
  ucs_info("Received %s", msg_host);
  return UCS_OK;
}
#endif
