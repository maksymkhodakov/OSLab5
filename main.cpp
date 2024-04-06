#include <sys/msg.h>
#include <sys/ipc.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

// Оголошення структури повідомлень
struct msgbuf {
    long mtype;       // тип повідомлення
    int mvalue;       // значення, що передається в повідомленні
};

// Функція, що відправляє результат обчислення в чергу повідомлень
void send_result(int msgid, int result, int result_type) {
    struct msgbuf msg;
    msg.mtype = result_type;
    msg.mvalue = result;
    msgsnd(msgid, &msg, sizeof(msg.mvalue), 0);
}

// Функція f(x), яка тепер повертає значення x замість вічного циклу
int f(int x) {
    // Імітація тривалої операції
    sleep(1);
    return x * 2; // Приклад обчислення
}

// Функція g(x), що затримується на 3 секунди
int g(int x) {
    sleep(3);
    return x * 3; // Приклад обчислення
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <integer value>\n", argv[0]);
        return -1;
    }

    int x = atoi(argv[1]);

    // Створення черги повідомлень
    int msgid = msgget(IPC_PRIVATE, 0666 | IPC_CREAT);

    if (fork() == 0) {
        // Дочірній процес для f(x)
        int result = f(x);
        send_result(msgid, result, 1);
        exit(0);
    }

    if (fork() == 0) {
        // Дочірній процес для g(x)
        int result = g(x);
        send_result(msgid, result, 2);
        exit(0);
    }

    // Очікування результатів від обох процесів
    struct msgbuf msg;
    int results[2] = {0}, count = 0;
    while (count < 2) {
        if (msgrcv(msgid, &msg, sizeof(msg.mvalue), 0, 0) > 0) {
            results[msg.mtype - 1] = msg.mvalue;
            count++;
        }
    }

    // Обчислення та виведення результату
    int product = results[0] * results[1];
    printf("Result: %d\n", product);

    // Видалення черги повідомлень
    msgctl(msgid, IPC_RMID, NULL);

    return 0;
}
