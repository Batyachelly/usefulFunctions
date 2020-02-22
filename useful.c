#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include <math.h>

#include "useful.h"

//! ПОЧИТАЙ ПРО ТО КАКОЙ ДЕФОЛТНЫЙ СТАНДАРТ Си В GCC

static int uVectMatrix_get_value_by_n(t_uVectMatrix *uVectMatrix, unsigned n,
                                      unsigned *value) {
  *value = *(uVectMatrix->vector + n);

  return 0;
}

static int uVectMatrix_set_value_by_n(t_uVectMatrix *uVectMatrix, unsigned n,
                                      unsigned value) {
  *(uVectMatrix->vector + n) = value;

  return 0;
}

static int uVectMatrix_get_value_by_cords(t_uVectMatrix *uVectMatrix,
                                          t_uCords *uCords, unsigned *value) {
  unsigned z_off = uCords->z * uVectMatrix->x_size * uVectMatrix->y_size;
  unsigned y_off = uCords->y * uVectMatrix->x_size;
  unsigned n = uCords->x + y_off + z_off;
  *value = *(uVectMatrix->vector + n);

  return 0;
}

static int uVectMatrix_set_value_by_cords(t_uVectMatrix *uVectMatrix,
                                          t_uCords *uCords, unsigned value) {
  unsigned z_off = uCords->z * uVectMatrix->x_size * uVectMatrix->y_size;
  unsigned y_off = uCords->y * uVectMatrix->x_size;
  unsigned n = uCords->x + y_off + z_off;
  *(uVectMatrix->vector + n) = value;

  return 0;
}

static unsigned *uVectMatrix_set_size(t_uVectMatrix *uVectMatrix, unsigned y,
                                      unsigned x, unsigned z) {
  uVectMatrix->y_size = y;
  uVectMatrix->x_size = x;
  uVectMatrix->z_size = z;

  return 0;
}

t_uVectMatrix *init_uVectMatrix() {
  t_uVectMatrix *uVectMatrix = (t_uVectMatrix *)malloc(sizeof(t_uVectMatrix));
  uVectMatrix->vector_size = VECTOR_SIZE;
  uVectMatrix->vector = (unsigned *)calloc(VECTOR_SIZE, sizeof(unsigned));
  uVectMatrix->get_value_by_cords = uVectMatrix_get_value_by_cords;
  uVectMatrix->set_value_by_cords = uVectMatrix_set_value_by_cords;
  uVectMatrix->get_value_by_n = uVectMatrix_get_value_by_n;
  uVectMatrix->set_value_by_n = uVectMatrix_set_value_by_n;
  uVectMatrix->set_size = uVectMatrix_set_size;

  return uVectMatrix;
}

////////////////////////////////////////////////////////////////////////////////

static int
uSocketTransmission_connect_as_server_unix(t_uSocketTransmission *uST) {
  int ret = 0;
  struct sockaddr_un sockaddr;

  memset(&sockaddr, 0, sizeof(struct sockaddr_un));
  sockaddr.sun_family = AF_UNIX;
  strncpy(sockaddr.sun_path, uST->socket_name, sizeof(sockaddr.sun_path) - 1);

  uST->socketfd = socket(AF_UNIX, SOCK_STREAM, 0);
  if (uST->socketfd == -1) {
    perror("socket");
    exit(EXIT_FAILURE);
  }

  ret = bind(uST->socketfd, (const struct sockaddr *)&sockaddr,
             sizeof(struct sockaddr_un));
  if (ret == -1) {
    perror("bind");
    exit(EXIT_FAILURE);
  }

  ret = listen(uST->socketfd, 10);
  if (ret == -1) {
    perror("listen");
    exit(EXIT_FAILURE);
  }

  uST->clientfd = accept(uST->socketfd, NULL, NULL);

  return uST->clientfd;
}

static int
uSocketTransmission_connect_as_client_unix(t_uSocketTransmission *uST) {
  int ret = 0;
  struct sockaddr_un sockaddr;

  memset(&sockaddr, 0, sizeof(struct sockaddr_un));
  sockaddr.sun_family = AF_UNIX;
  strncpy(sockaddr.sun_path, uST->socket_name, sizeof(sockaddr.sun_path) - 1);

  uST->clientfd = socket(AF_UNIX, SOCK_STREAM, 0);
  if (uST->clientfd == -1) {
    perror("socket");
    exit(EXIT_FAILURE);
  }

  ret = connect(uST->clientfd, (const struct sockaddr *)&sockaddr,
                sizeof(struct sockaddr_un));
  if (ret == -1) {
    fprintf(stderr, "The server is down.\n");
    exit(EXIT_FAILURE);
  }

  return ret;
}

static int
uSocketTransmission_connect_as_server_inet(t_uSocketTransmission *uST) {
  int ret = 0;
  struct sockaddr_in sockaddr;

  memset(&sockaddr, 0, sizeof(struct sockaddr_in));
  sockaddr.sin_family = AF_INET;
  sockaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  sockaddr.sin_port = htons(5000);

  uST->socketfd = socket(AF_INET, SOCK_STREAM, 0);
  if (uST->socketfd == -1) {
    perror("socket");
    exit(EXIT_FAILURE);
  }

  ret = bind(uST->socketfd, (const struct sockaddr *)&sockaddr,
             sizeof(struct sockaddr_in));
  if (ret == -1) {
    perror("bind");
    exit(EXIT_FAILURE);
  }

  ret = listen(uST->socketfd, 10);
  if (ret == -1) {
    perror("listen");
    exit(EXIT_FAILURE);
  }

  uST->clientfd = accept(uST->socketfd, NULL, NULL);

  return uST->clientfd;
}

static int
uSocketTransmission_connect_as_client_inet(t_uSocketTransmission *uST) {
  int ret = 0;
  struct sockaddr_in sockaddr;

  memset(&sockaddr, 0, sizeof(struct sockaddr_in));
  sockaddr.sin_family = AF_INET;
  sockaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  sockaddr.sin_port = htons(5000);

  uST->clientfd = socket(AF_INET, SOCK_STREAM, 0);
  if (uST->clientfd == -1) {
    perror("socket");
    exit(EXIT_FAILURE);
  }

  ret = connect(uST->clientfd, (const struct sockaddr *)&sockaddr,
                sizeof(struct sockaddr_in));
  if (ret == -1) {
    fprintf(stderr, "The server is down.\n");
    exit(EXIT_FAILURE);
  }

  return ret;
}

static int uSocketTransmission_write(t_uSocketTransmission *uST, char *data) {
  int ret = 0;
  ret = (ret = write(uST->clientfd, data, uST->frame_size));

  return ret;
}

static int uSocketTransmission_read(t_uSocketTransmission *uST, char *data) {
  int ret = 0;
  memset(uST->buffer, 0, uST->frame_size);
  if (-1 != (ret = read(uST->clientfd, uST->buffer, uST->frame_size)))
    memcpy(data, uST->buffer, uST->frame_size);

  return ret;
}

static int uSocketTransmission_write_VectMatrix(t_uSocketTransmission *uST,
                                                t_uVectMatrix *uVM) {
  int ret = 0;
  int parts_num = ceil((uVM->vector_size * sizeof(unsigned)) / uST->frame_size);
  int part_size = uST->frame_size / sizeof(unsigned);
  for (size_t i = 0; i < parts_num; i++) {
    memset(uST->buffer, 0, uST->frame_size);
    memcpy(uST->buffer, uVM->vector + i * part_size, uST->frame_size);
    uST->write(uST, uST->buffer);
  }

  return ret;
}

static int uSocketTransmission_read_VectMatrix(t_uSocketTransmission *uST,
                                               t_uVectMatrix *uVM) {
  int ret = 0;
  int parts_num = ceil((uVM->vector_size * sizeof(unsigned)) / uST->frame_size);
  int part_size = uST->frame_size / sizeof(unsigned);
  for (size_t i = 0; i < parts_num; i++) {
    memset(uST->buffer, 0, uST->frame_size);
    uST->read(uST, uST->buffer);
    memcpy(uVM->vector + i * part_size, uST->buffer, uST->frame_size);
  }

  return ret;
}

t_uSocketTransmission *init_uSocketTransmission(char *socket_name) {
  t_uSocketTransmission *uST =
      (t_uSocketTransmission *)malloc(sizeof(t_uSocketTransmission));
  uST->socket_name = (char *)calloc(SOCK_NAME_SIZE, sizeof(char));
  strncpy(uST->socket_name, socket_name, strlen(socket_name));
  uST->buffer = (char *)calloc(FRAME_SIZE, sizeof(char));
  uST->frame_size = FRAME_SIZE;
  uST->connect_as_server_unix = uSocketTransmission_connect_as_server_unix;
  uST->connect_as_client_unix = uSocketTransmission_connect_as_client_unix;
  uST->connect_as_server_inet = uSocketTransmission_connect_as_server_inet;
  uST->connect_as_client_inet = uSocketTransmission_connect_as_client_inet;
  uST->write = uSocketTransmission_write;
  uST->read = uSocketTransmission_read;
  uST->write_VectMatrix = uSocketTransmission_write_VectMatrix;
  uST->read_VectMatrix = uSocketTransmission_read_VectMatrix;

  return uST;
}