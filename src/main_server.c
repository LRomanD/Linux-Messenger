#include "../hdr/network.h"

#define QUEUE_SIZE                      20

struct servers_list_t
{
    int thread_id;
    struct server_info_t server_info;
};

struct server_thread_t *threads = NULL;
int threads_count = 0;
pthread_mutex_t threads_mutex = PTHREAD_MUTEX_INITIALIZER;

struct servers_list_t *servers_list = NULL;
int servers_count = 0;
pthread_mutex_t servers_list_mutex = PTHREAD_MUTEX_INITIALIZER;

static void sigint_handler(int sig, siginfo_t *si, void *unused)
{
    exit(EXIT_SUCCESS);
}

void shutdown_server(void)
{
    if (threads != NULL)
    {
        int index;

        for (index = 0; index < threads_count; index++)
        {
            pthread_cancel(threads[index].tid);
            if (threads[index].fd > 0)
                close(threads[index].fd);
        }

        free(threads);
    }

    if (servers_list != NULL)
    {
        free(servers_list);
    }

    unlink("/free_threads_sem");
    unlink("/busy_threads_sem");
    unlink("/main_server_fds");

    puts("Server shutdown");
}

int _broadcast_message(struct client_msg_t *msg)
{

    return EXIT_SUCCESS;
}

int _cast_message_by_addr(struct client_msg_t *msg, char *ip, unsigned short port)
{
    return EXIT_SUCCESS;
}

void *_processing_server_thread(void *args)
{
	struct pollfd pfd;
	struct client_msg_t msg;
    mqd_t fds_q;
    char queue_msg[QUEUE_SIZE+1];
    sem_t *free_threads_sem;
    sem_t *busy_threads_sem;

    int thread_id = (int)args;
    int client_type = TYPE_NONE;
    int client_id = 0;
    char client_name[NAME_LEN+1];

    int ret = 0;

    fds_q = mq_open("/main_server_fds", O_RDONLY);
    if (fds_q == -1)
    {
        perror("mq_open");
    }

	pfd.events = POLLIN;

	/* Set canceltype so thread could be canceled at any time*/
	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

    /* Wait for parent to create sem segments, then get it's fds */
	while ((free_threads_sem = sem_open("/free_threads_sem", O_RDWR)) == SEM_FAILED)
	{
		if (errno != ENOENT)
		{
			perror("free_threads_sem sem_open");
			exit(EXIT_FAILURE);
		}
	}
	while ((busy_threads_sem = sem_open("/busy_threads_sem", O_RDWR)) == SEM_FAILED)
	{
		if (errno != ENOENT)
		{
			perror("busy_threads_sem sem_open");
			exit(EXIT_FAILURE);
		}
	}

    sem_post(free_threads_sem);

    while(1)
    {
        if (mq_receive(fds_q, queue_msg, QUEUE_SIZE, NULL) == -1)
        {
            perror("mq_receive");
            continue;
        }
        pfd.fd = strtol(queue_msg, NULL, 0);

        sem_wait(free_threads_sem);
        sem_post(busy_threads_sem);

        pthread_mutex_lock(&threads_mutex);
        pthread_mutex_unlock(&threads_mutex);

        pthread_mutex_lock(&threads[thread_id].mutex);
        threads[thread_id].fd = pfd.fd;
        pthread_mutex_unlock(&threads[thread_id].mutex);

        printf("Processing server #%d will read the %dth fd\n", thread_id, pfd.fd);

        while(1)
        {
            if (poll(&pfd, 1, 0) > 0)
            {
                if (pfd.revents & POLLIN)
                {
                    ret = recv(pfd.fd, &msg, sizeof(msg), 0);
                    if (ret == -1)
                    {
                        perror("recv");
                        break;
                    }
                    else if (ret == 0)
                    {
                        puts("Client disconnected");
                        break;
                    }
                    else
                    {
                        if (msg.command == PING_COMM)
                        {
                            puts("PING command received");
                            msg.command = PING_ANSW;

                            if (send(pfd.fd, &msg, sizeof(msg), 0) == -1)
                            {
                                perror("send");
                                break;
                            }
                        }
                        else if (msg.command == CONNECT_COMM)
                        {
                            printf("User: %s(%d) connected\n", msg.client_info.client_name, msg.client_info.id);

                            client_type = TYPE_USER;
                            client_id = 1;
                            strncpy(client_name, "NewUser", NAME_LEN);

                            msg.command = CONNECT_ANSW;
                            strncpy(msg.client_info.client_name, client_name, NAME_LEN);
                            msg.client_info.id = client_id;
                            msg.client_info.client_type = client_type;
                            msg.client_info.cur_server = 0;

                            if (send(pfd.fd, &msg, sizeof(msg), 0) == -1)
                            {
                                perror("send");
                                break;
                            }
                        }
                        else if (msg.command == JOIN_COMM)
                        {
                            printf("User: %s(%d) attempting to join server %s:%d\n", msg.join_srv.client_name, msg.join_srv.usr_id, msg.join_srv.ip, msg.join_srv.port);

                            // find server with this address and send connect request to it, change client state if attempt was success
                        }
                        else if (msg.command == CREATE_COMM)
                        {
                            printf("User %s #%d creating a server <%s>, address: %s:%d\n", client_name, msg.server_info.host_id, msg.server_info.server_name, msg.server_info.ip, msg.server_info.port);

                            // check if given address is free, allocate memory to server, send request
                        }
                        else if (msg.command == RENAME_COMM)
                        {
                            printf("User %s #%d requested rename operation to <%s>\n", client_name, msg.client_info.id, msg.client_info.client_name);


                        }
                        else if (msg.command == DISCONNECT_COMM)
                        {
                            if (client_type != TYPE_SERVER)
                                break;
                            msg.command = DISCONNECT_ANSW;

                            if (send(pfd.fd, &msg, sizeof(msg), 0) == -1)
                            {
                                perror("send");
                                break;
                            }

                            puts("Shut down processing server");
                            break;
                        }
                        else if (msg.command == CLIENT_QUIT_COMM)
                        {
                            printf("User: %s(%d) disconnected\n", msg.client_info.client_name, msg.client_info.id);

                            msg.command = CLIENT_QUIT_ANSW;

                            if (send(pfd.fd, &msg, sizeof(msg), 0) == -1)
                            {
                                perror("send");
                                break;
                            }
                        }
                        else if (msg.command == SHUT_ROOM_COMM)
                        {
                            printf("User %s #%d shutting a server <%s>, address: %s:%d\n", client_name, msg.server_info.host_id, msg.server_info.server_name, msg.server_info.ip, msg.server_info.port);
                        }
                        else if (msg.command == SHUT_SRV_COMM)
                        {
                            msg.command = SHUT_SRV_ANSW;

                            if (send(pfd.fd, &msg, sizeof(msg), 0) == -1)
                            {
                                perror("send");
                                break;
                            }

                            puts("Shut down processing server");
                            break;
                        }
                    }
                    pfd.revents = 0;
                }
                else if (pfd.revents & POLLHUP)
                {
                    puts("FD Closed");
                    break;
                }
            }
        }
        sem_wait(busy_threads_sem);
        sem_post(free_threads_sem);

        pthread_mutex_lock(&threads_mutex);
        pthread_mutex_unlock(&threads_mutex);

        pthread_mutex_lock(&threads[thread_id].mutex);
        threads[thread_id].fd = 0;
        pthread_mutex_unlock(&threads[thread_id].mutex);
    }
	
    mq_close(fds_q);
    exit(EXIT_SUCCESS);
}

int main_server()
{
    int server_fd;
    struct sockaddr_in server;
    int tmp_fd = 0;
    struct pollfd *client_pfds = NULL;
    struct sockaddr_in client;
    int client_size;

    struct sigaction sa;

    sem_t *free_threads_sem;
    sem_t *busy_threads_sem;
    int sem_value = 0;

    mqd_t fds_q;
    mqd_t *broadcast_q;
    struct mq_attr attr;

    struct client_msg_t msg;
    char queue_msg[QUEUE_SIZE+1];

    int index;
    int srvs_num = 0;
    int ret = 0;

    sigemptyset(&sa.sa_mask);
    sa.sa_sigaction = sigint_handler;
    if (sigaction(SIGINT, &sa, NULL) == -1)
    {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }

    if (atexit(shutdown_server) != 0)
    {
        perror("atexit");
        exit(EXIT_FAILURE);
    }

    free_threads_sem = sem_open("/free_threads_sem", O_CREAT | O_RDWR, 0666, 0);
	if (free_threads_sem == SEM_FAILED)
	{
		perror("free_threads_sem sem_open");
		exit(EXIT_FAILURE);
	}
	busy_threads_sem = sem_open("/busy_threads_sem", O_CREAT | O_RDWR, 0666, 0);
	if (busy_threads_sem == SEM_FAILED)
	{
		perror("busy_threads_sem sem_open");
		exit(EXIT_FAILURE);
	}

    sem_getvalue(free_threads_sem, &sem_value);
	while (sem_value > 0)
	{
		sem_trywait(free_threads_sem);
		sem_getvalue(free_threads_sem, &sem_value);
	}
    sem_getvalue(busy_threads_sem, &sem_value);
	while (sem_value > 0)
	{
		sem_trywait(busy_threads_sem);
		sem_getvalue(busy_threads_sem, &sem_value);
	}

    pthread_mutex_lock(&threads_mutex);
    threads_count = SERVER_THREADS_ALLOC;
    threads = malloc(threads_count * sizeof(struct server_thread_t));
    if (threads == NULL)
    {
        printf("malloc: %s(%d)\n", strerror(errno), errno);
        exit(EXIT_FAILURE);
    }

    for (index = 0; index < threads_count; index++)
    {
        threads[index].tid = NULL;
        pthread_create(&threads[index].tid, NULL, _processing_server_thread, index);
        threads[index].fd = 0;
        pthread_mutex_init(&threads[index].mutex, NULL);
    }
    pthread_mutex_unlock(&threads_mutex);

    attr.mq_maxmsg = 5;
	attr.mq_msgsize = sizeof(char) * QUEUE_SIZE;

    fds_q = mq_open("/main_server_fds", O_CREAT | O_RDWR, 0666, &attr);
    if (fds_q == -1)
    {
        perror("mq_open");
        exit(EXIT_FAILURE);
    }

    /* Fill 'client_pfds' and 'server' with 0's */
	memset(&server, 0, sizeof(server));
    memset(&queue_msg, 0, sizeof(queue_msg)/sizeof(char));

	/* Set server's endpoint */
	server.sin_family = AF_INET;
	if (inet_pton(AF_INET, config.ip, &server.sin_addr) == -1)
	{
        printf("inet_pton: %s(%d)\n", strerror(errno), errno);
        exit(EXIT_FAILURE);
	}
	server.sin_port = config.port;

    printf("Server addr: %s : %d\n", config.ip, config.port);

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd > 0)
    {
        setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int));

        if (bind(server_fd, (struct sockaddr *)&server, sizeof(server)) == -1)
		{
            printf("bind: %s(%d)\n", strerror(errno), errno);
			exit(EXIT_FAILURE);
		}

        if (listen(server_fd, SERVER_LISTEN_BACKLOG) == -1)
		{
            printf("listen: %s(%d)\n", strerror(errno), errno);
			exit(EXIT_FAILURE);
		}

        while(1)
        {
            client_size = sizeof(client);
            if ((tmp_fd = accept(server_fd, (struct sockaddr *)&client,
                                    &client_size)) == -1)
            {
                printf("accept: %s(%d)\n", strerror(errno), errno);
                exit(EXIT_FAILURE);
            }

            if (sem_getvalue(free_threads_sem, &sem_value) != 0)
            {
                perror("sem_getvalue");
                exit(EXIT_FAILURE);
            }

            if (sem_value == 0)
            {
                if (sem_getvalue(busy_threads_sem, &sem_value) != 0)
                {
                    perror("sem_getvalue");
                    exit(EXIT_FAILURE);
                }
                if (sem_value == threads_count)
                {
                    struct server_thread_t *tmp;
                    tmp = realloc(threads, (threads_count+SERVER_THREADS_ALLOC) * sizeof(struct server_thread_t));
                    if (tmp == NULL)
                    {
                        perror("realloc");
                        exit(EXIT_FAILURE);
                    }

                    pthread_mutex_lock(&threads_mutex);
                    threads = tmp;
                    tmp = NULL;

                    for (index = threads_count; index < (threads_count+SERVER_THREADS_ALLOC); index++)
                    {
                        threads[index].tid = NULL;
                        pthread_create(&threads[index].tid, NULL, _processing_server_thread, index);
                        threads[index].fd = 0;
                        pthread_mutex_init(&threads[index].mutex, NULL);
                    }

                    threads_count+=SERVER_THREADS_ALLOC;
                    pthread_mutex_unlock(&threads_mutex);
                }
            }

            snprintf(queue_msg, QUEUE_SIZE, "%d", tmp_fd);
            while (mq_send(fds_q, queue_msg, sizeof(QUEUE_SIZE), NULL) == -1)
            {
                if (errno != EAGAIN)
                {
                    perror("mq_send");
                    exit(EXIT_FAILURE);
                }
            }
            // if (mq_send(fds_q, queue_msg, sizeof(QUEUE_SIZE), NULL) != 0)
            // {
            //     perror("mq_send");
            //     exit(EXIT_FAILURE);
            // }
        }
    }
    else
    {
        printf("socket: %s(%d)\n", strerror(errno), errno);
        exit(EXIT_FAILURE);
    }

    mq_close(fds_q);

    return ret;
}