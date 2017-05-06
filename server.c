#include <assert.h>
#include <getopt.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include "server.h"

#include <signal.h>
#include <sys/wait.h>
//inet_ntoa
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

//char const gprogram_name[] = "server";
char const* program_name = "server";
//xtern char const * program_name;
int verbose = 0;
/* Описание длинных опций для функции getopt_long(). */
static const struct option long_options[] = {
	{ "address", 1, NULL, 'a'},
	{ "help", 0, NULL, 'h'},
	{ "process", 0, NULL, 'r'},
	{ "thread", 0, NULL, 't'},
	{ "port", 1, NULL, 'p'},
	{ "verbose", 0, NULL, 'v'}
};

/* Описание коротких опций для функции getopt_long(). */
static const char* const short_options = "a:hrtp:v";

/* Сообщение о том, как правильно использовать программу. */
static const char* const usage_template =
	"Usage: %s { options }\n"
	" -a, --address ADDR Bind to local address (by default, bind\n"
	" to all local addresses).\n"
	" -h, --help Print this information.\n"
	" -r -process    process for each client\n"
	" -t -thread    thread for each client\n"
	" -p, --port PORT Bind to specified port.\n"
	" -v, --verbose Print verbose messages.\n";

/* Вывод сообщения о правильном использовании программы
 и завершение работы. Если аргумент IS_ERROR не равен нулю,
 сообщение записывается в поток stderr и возвращается
 признак ошибки, в противном случае сообщение выводится в
 поток stdout и возвращается обычный нулевой код. */
static void print_usage(int is_error)
{
	fprintf(is_error ? stderr : stdout, usage_template,
		program_name);
	exit(is_error ? 1 : 0);
}
/* Обработчик сигнала SIGCHLD, удаляющий завершившиеся
 дочерние процессы. */
static void clean_up_child_process(int signal_number)
{
	int status;
	wait(&status);
}

/* HTTP-ответ и заголовок, возвращаемые в случае
 успешной обработки запроса.*/
static char const* const ok_response =
	" OK!\n"; 

/* ответ,на случай непонятного запроса. */
static char const* const bad_request_response =
	"Bad Reguest\n";
/* Обработка запроса  PAGE и
 запись результата в файл с дескриптором CONNECTION_FD. */
static void handle_get(int connection_fd, const char* message)
{

	/* Убеждаемся, что имя страницы начинается с косой черты и
	не содержит других символов косой черты, так как
	подкаталоги не поддерживаются. */
	char response[1024];

	if (message == NULL) {
		/* Имя страницы неправильно сформировано или не удалось
		открыть модуль с указанным именем. В любом случае
		возвращается HTTP-ответ "404. Not Found". */

		/* Формирование ответного сообщения. */
		snprintf(response, sizeof(response),
			bad_request_response);
		/* Отправка его клиенту. */
		write(connection_fd, response, strlen(response));
	} else {
		/* Запрашиваемый модуль успешно загружен. */

		snprintf(response, sizeof(response),
			" recieve: %s, '%s' . ", ok_response, message);
		/* Выдача HTTP-ответа, обозначающего успешную обработку
		запроса, и HTTP-заголовка для HTML-страницы. */
		write(connection_fd, response, strlen(response));
		/* Вызов модуля, генерирующего HTML-код страницы и
		записывающего этот код в указанный файл. */
		//(*module->generate_function)(connection_fd);
		/* Работа с модулем окончена. */
		//module_close(module);
	}
}

/* Обработка клиентского запроса на подключение. */
static void handle_connection(int connection_fd)
{
	char buffer[256];
	ssize_t bytes_read;

	/* Получение данных от клиента. */
	bytes_read =
		read(connection_fd, buffer, sizeof(buffer) - 1);
	if (bytes_read > 0) {

		//char protocol[sizeof(buffer)];

		/* Часть данных успешно прочитана. Завершаем буфер
		нулевым символом, чтобы его можно было использовать
		в строковых операциях. */
		//buffer[bytes_read] = '\0';
		/*  строка, посылаемая клиентом, -- это запрос.
		 */
		printf(" server:Recieve : '%s'", buffer);
		
		//sscanf(buffer, "%s", protocol);
		/* В заголовке, стоящем после запроса, может находиться
		любая информация. В данной реализации HTTP-сервера
		эта информация не учитывается. Тем не менее необходимо
		прочитать все данные, посылаемые клиентом. Данные читаются
		до тех пор, пока не встретится конец заголовка,
		обозначаемый пустой строкой. В HTTP пустой строке
		соответствуют символы CR/LF. */
		//while (strstr(buffer, " \r\n\r\n") == NULL)
		//	bytes_read = read(connection_fd, buffer, sizeof(buffer));
		/* Проверка правильности последней операции чтения.
		Если она не завершилась успешно, произошел разрыв
		соединения, поэтому завершаем работу. */
		if (bytes_read == -1) {
			close(connection_fd);
			return;
		}
		/* Проверка поля версии. Сервер понимает протокол HTTP
		версий 1.0 и 1.1. */
		if (0) {
			/* Протокол не поддерживается. */
			write(connection_fd, bad_request_response,
				sizeof(bad_request_response));
		} else
			/* Корректный запрос. Обрабатываем его. */
			handle_get(connection_fd, buffer);
	} else if (bytes_read == 0)
		/* Клиент разорвал соединение, не успев отправить данные.
		Ничего не предпринимаем */
		;
	else
		/* Операция чтения завершилась ошибкой. */
		system_error("read");
}
int do_exit = 0;
void terminator_sig_hndlr(int sn)
{
	printf("\n terminator_sig_hndlr : %d.\n", sn);
	do_exit = 1;
	//if (-1 != server_socket)
	//	close(server_socket);
}
void server_run(struct in_addr local_address, uint16_t port)
{
	struct sockaddr_in socket_address;
	int rval;
	struct sigaction sigchld_action;
	int server_socket = -1;	
	/* Устанавливаем обработчик сигнала SIGCHLD, который будет
	удалять завершившееся дочерние процессы. */
	memset(&sigchld_action, 0, sizeof(sigchld_action));
	sigchld_action.sa_handler = &clean_up_child_process;
	sigaction(SIGCHLD, &sigchld_action, NULL);

	/* Создание TCP-сокета */
	server_socket = socket(PF_INET, SOCK_STREAM, 0);
	if (server_socket == -1) system_error("socket");
	
		
	/**/struct sigaction sa;
	memset(&sa, 0, sizeof(sa));
	sigemptyset(&sa.sa_mask);
	
	sa.sa_handler = &terminator_sig_hndlr;
	sigaction(SIGTERM, &sa, NULL);
	sa.sa_handler = &terminator_sig_hndlr;
	sigaction(SIGINT, &sa, NULL);
	

	/* Создание адресной структуры, определяющей адрес
	для приема запросов. */
	memset(&socket_address, 0, sizeof(socket_address));
	socket_address.sin_family = AF_INET;
	socket_address.sin_port = port;
	socket_address.sin_addr = local_address;
	/* Привязка сокета к этому адресу. */
	rval =
		bind(server_socket, &socket_address,
		sizeof(socket_address));
	if (rval != 0)
		system_error("bind");
	/* Перевод сокета в режим приема запросов. */
	rval = listen(server_socket, 10);
	if (rval != 0)
		system_error("listen");

	if (verbose) {
		/* В режиме развернутых сообщений отображаем адрес и порт,
		с которыми работает сервер. */
		socklen_t address_length;

		/* Нахождение адреса сокета. */
		address_length = sizeof(socket_address);
		rval =
			getsockname(server_socket, &socket_address, &address_length);
		assert(rval == 0);
		/* Вывод сообщения. Номер порта должен быть преобразован
		из сетевого (обратного) порядка следования байтов
		в серверный (прямой). */
		printf("server listening on %s:%d\n",
			inet_ntoa(socket_address.sin_addr),
			(int) ntohs(socket_address.sin_port));
	}

	/* Бесконечный цикл обработки запросов. */
	while (!do_exit) {
		struct sockaddr_in remote_address;
		socklen_t address_length;
		int connection;
		pid_t child_pid;

		/* Прием запроса. Эта функция блокируется до тех пор, пока
		не поступит запрос. */
		address_length = sizeof(remote_address);
		errno = 0;
		connection = accept(server_socket, &remote_address,
			&address_length);
		if (connection == -1) {
			/* Функция завершилась неудачно. */
			if (errno == EINTR)
				/* Функция была прервана сигналом. Повторная попытка. */
				continue;
			else
				/* Что-то случилось. */
				system_error("accept");
		}

		/* Соединение установлено. Вывод сообщения, если сервер
		работает в режиме развернутых сообщений. */
		if (verbose) {
			socklen_t address_length;
			/* Получение адреса клиента. */
			address_length = sizeof(socket_address);
			rval =
				getpeername(connection, &socket_address, &address_length);
			assert(rval == 0);
			/* Вывод сообщения. */
			printf("connection accepted from %s\n",
				inet_ntoa(socket_address.sin_addr));
		}

		/* Создание дочернего процесса для обработки запроса. */
		child_pid = fork();
		if (child_pid == 0) {
			printf(" child proc: %d\n", getpid());
			/* Это дочерний процесс. Потоки stdin и stdout ему не нужны,
			поэтому закрываем их. */

			close(STDIN_FILENO);
			close(STDOUT_FILENO);
			/* Дочерний процесс не должен работать с серверным сокетом,
			поэтому закрываем его дескриптор. */
			close(server_socket);
			/* Обработка запроса. */
			handle_connection(connection);
			/* Обработка завершена. Закрываем соединение и завершаем
			дочерний процесс. */
			close(connection);
			exit(0);
		} else if (child_pid > 0) {
			/* Это родительский процесс. Дескриптор клиентского сокета
			ему не нужен. Переход к приему следующего запроса. */
			close(connection);
		} else
			/* Вызов функции fork() завершился неудачей. */
			system_error("fork");
	}
	puts("server is closing");
	close(server_socket);
	server_socket = -1;
}

int main(int argc, char* const argv[])
{
	struct in_addr local_address;
	uint16_t port;
	int next_option;


	/* Сохранение имени программы для отображения в сообщениях
	об ошибке. */
	program_name = argv[0];

	/* Назначение стандартных установок. По умолчанию сервер
	связан со всеми локальными адресами, и ему автоматически
	назначается неиспользуемый порт. */
	local_address.s_addr = INADDR_ANY;
	port = (uint16_t) htons(TCP_PORT);
	/*  отображать развернутые сообщения. */
	verbose = 1;

	/* Анализ опций. */
	do {
		next_option =
			getopt_long(argc, argv, short_options,
			long_options, NULL);
		switch (next_option) {
		case 'a':
			/* Пользователь ввел -a или --address. */
		{
			struct hostent* local_host_name;

			/* Поиск заданного адреса. */
			local_host_name = gethostbyname(optarg);
			if (local_host_name == NULL ||
				local_host_name->h_length == 0)
				/* He удалось распознать имя. */
				error(optarg, "invalid host name");
			else
				/* Введено правильное имя */
				local_address.s_addr =
				*((int*) (local_host_name->h_addr_list[0]));
		}
			break;
		case 'h':
			/* Пользователь ввёл -h или --help. */
			print_usage(0);
		case 't':
			/* Пользователь ввел -t или --thread. */
		{
			printf("thread not supported\n");
			exit(1);
		}
			break;
		case 'r':
			/* Пользователь ввел -r или --process. */
		{
			printf("process mode\n");
		}
			break;
		case 'p':
			/* Пользователь ввел -p или --port. */
		{
			long value;
			char* end;

			value = strtol(optarg, &end, 10);
			if (*end != '\0')
				/* В номере порта указаны не только цифры. */
				print_usage(1);
			/* Преобразуем номер порта в число с сетевым (обратным)
			порядком следования байтов. */
			port = (uint16_t) htons(value);
		}
			break;
		case 'v':
			/* Пользователь ввел -v или --verbose. */
			verbose = 1;
			break;
		case '?':
			/* Пользователь ввел непонятную опцию. */
			print_usage(1);
		case -1:
			/* Обработка опций завершена. */
			break;
		default:
			abort();
		}
	} while (next_option != -1);

	/* Программа не принимает никаких дополнительных аргументов.
	Если они есть, выдается сообщение об ошибке. */
	if (optind != argc)
		print_usage(1);

	/* Отображение имени каталога, если программа работает в режиме
	развернутых сообщений. */

	/* Запуск сервера. */
	server_run(local_address, port);
	return 0;
}
