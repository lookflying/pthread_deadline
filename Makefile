pthread_deadline: pthread_deadline.c
	gcc -o pthread_deadline pthread_deadline.c -lpthread
clean:
	rm -rf *.o pthread_deadline
