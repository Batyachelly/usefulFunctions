
////////////////////////////////////////////////////////////////////////////////

#define VECTOR_SIZE 51200

typedef struct uCords t_uCords;
struct uCords {
  unsigned y;
  unsigned x;
  unsigned z;
};

typedef struct uVectMatrix t_uVectMatrix;
struct uVectMatrix {
  unsigned *vector;
  unsigned vector_size;
  unsigned y_size, x_size, z_size;
  int (*get_value_by_n)(t_uVectMatrix *, unsigned, unsigned *);
  int (*set_value_by_n)(t_uVectMatrix *, unsigned, unsigned);
  int (*get_value_by_cords)(t_uVectMatrix *, t_uCords *, unsigned *);
  int (*set_value_by_cords)(t_uVectMatrix *, t_uCords *, unsigned);
  unsigned *(*set_size)(t_uVectMatrix *, unsigned, unsigned, unsigned);
};
t_uVectMatrix *init_uVectMatrix();

////////////////////////////////////////////////////////////////////////////////

#define FRAME_SIZE 512
#define SOCK_NAME_SIZE 50

typedef struct uSocketTransmission t_uSocketTransmission;
struct uSocketTransmission {
  char *socket_name;
  char *buffer;
  unsigned socketfd;
  unsigned clientfd;
  unsigned frame_size;
  int (*connect_as_server_unix)(t_uSocketTransmission *);
  int (*connect_as_client_unix)(t_uSocketTransmission *);
  int (*connect_as_server_inet)(t_uSocketTransmission *);
  int (*connect_as_client_inet)(t_uSocketTransmission *);
  int (*write)(t_uSocketTransmission *, char *);
  int (*read)(t_uSocketTransmission *, char *);
  int (*write_VectMatrix)(t_uSocketTransmission *, t_uVectMatrix *);
  int (*read_VectMatrix)(t_uSocketTransmission *, t_uVectMatrix *);
};
t_uSocketTransmission *init_uSocketTransmission(char *);

////////////////////////////////////////////////////////////////////////////////