#define CONNECTION_LIMIT 3
#define FREE_CONNECTION -1
#define BUF_SIZE 4096 /* Maximo tamaño del bufer para los flujos de datos que pueden leer el cliente y el servidor */

int sendResultClientUdp(int acceptfd, char* answer, int numRead, const struct sockaddr *claddr, socklen_t len);

int sendResultClient(int acceptfd, char* answer);

void sendResultServer(int acceptfd, char* command, char* answer, int resultWrite, int clients[]);

void sendNoticeReceiver(int acceptfd, int receivingDepartmentId, char* answer);

void invalidCommand(int acceptfd, char* invalidCommand, int clients[]);

void responseInvalidCommand(int acceptfd, char* answer, char* invalidCommand, int clients[]);

bool existDepartment(int idDepartment);

bool connectedClient(int idDepartment, int clients[]);

bool equalDepartment(int senderfd, int idDepartment, int clients[]);

void printConnections(int *amountConnections);

int getIdDepartment(int clients[], int acceptfd);

void resetCharArray(char buf[]);

void addClient(int connectionfd, int *amountConnections, pthread_mutex_t lock, int clients[]);

void removeClient(int clients[], int connectionfd, int *amountConnections);

void concatenateTextNotification(char* notification, char* firstPartNotification, char* secondPartNotification, int senderDepartmentId);

void fill(int clients[]);

char convertIntToChar(int number);

int getFdSocketTcp(char* ip, char* port);

int getFdSocketUdp(char* ip, char* port);
