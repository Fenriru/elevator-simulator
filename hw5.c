#include "elevator.h"

/*
About this:
starter code without a single semaphore
*/


#include <assert.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <semaphore.h>


sem_t sem;

static struct passenger {
    int from_floor;
    int to_floor;
    int elevator_id;
    //sem_t passenger_ready_for_pickup;
    sem_t passenger_in_elevator;
    sem_t passenger_exited_elevator;
    
    sem_t elevator_at_pickup;
    sem_t elevator_at_destination;  
} Passengers[PASSENGERS];

//int passenger_ready_for_pickup;
//int passenger_in_elevator;
//int passenger_exited_elevator;

//int elevator_at_pickup;
//int elevator_at_destination;

//sem_t passenger_ready_for_pickup;
//sem_t passenger_in_elevator;
//sem_t passenger_exited_elevator;

//sem_t elevator_at_pickup;
//sem_t elevator_at_destination;

//sem_t elevator_move;

static struct queue {
	int data[PASSENGERS];
	int front;
	int back;
	int size;
	sem_t lock;
} Queue;

static void add_to_queue(int passenger_id) {
	sem_wait(&Queue.lock);
	Queue.data[Queue.back] = passenger_id;
	Queue.back = (Queue.back + 1) % PASSENGERS;
	Queue.size++;
	sem_post(&Queue.lock);
}

static int remove_from_queue(void) {
	sem_wait(&Queue.lock);
	if (Queue.size <= 0) {
		sem_post(&Queue.lock);
		return -1;
	}
	int passenger_id = Queue.data[Queue.front];
	Queue.front = (Queue.front + 1) % PASSENGERS;
	Queue.size--;
	sem_post(&Queue.lock);
	return passenger_id;
}

void scheduler_init(void)
{
    memset(&Passengers, 0x00, sizeof(Passengers));
    memset(&Queue, 0x00, sizeof(Queue));
    sem_init(&Queue.lock, 0, 1);
    //memset(&Passenger, 0x00, sizeof(Passenger));
    //passenger_ready_for_pickup = 0;
    //passenger_in_elevator = 0;
    //passenger_exited_elevator = 0;
    //elevator_at_pickup = 0;
    //elevator_at_destination = 0;
    
    //sem_init(&passenger_ready_for_pickup, 0, 0);
    //sem_init(&passenger_in_elevator, 0, 0);
    //sem_init(&passenger_exited_elevator, 0, 0);
    //sem_init(&elevator_at_pickup, 0, 0);
    //sem_init(&elevator_at_destination, 0, 0);
    
    //sem_init(&elevator_move, 0, 1);
    //Passenger.curr = 0;
}


void passenger_request(int passenger, int from_floor, int to_floor,
                       void (*enter)(int, int), void (*exit)(int, int))
{
    struct passenger *p = &Passengers[passenger];

    // inform elevator of floor
    //sem_wait(&elevator_move);
    p->from_floor = from_floor;
    p->to_floor = to_floor;

    add_to_queue(passenger);
    
    //Passenger.curr = 1;
    //sem_post(&elevator_move);
    // signal ready and wait
    //passenger_ready_for_pickup = 1;
    //while (!elevator_at_pickup);
    //sem_post(&p->passenger_ready_for_pickup);
    sem_wait(&p->elevator_at_pickup);
    
    int elevator = p->elevator_id;    

    // enter elevator and wait
    enter(passenger, elevator);
    //passenger_in_elevator = 1;
    //while (!elevator_at_destination);
    sem_post(&p->passenger_in_elevator);
    sem_wait(&p->elevator_at_destination);

    // exit elevator and signal
    exit(passenger, elevator);
    //passenger_exited_elevator = 1;
    //Passenger.curr = 0;
    sem_post(&p->passenger_exited_elevator);
}


// example procedure that makes it easier to work with the API
// move elevator from source floor to destination floor
// you will probably have to modify this in the future ...

static void move_elevator(int elevator, int source, int destination, void (*move_direction)(int, int))
{
    //sem_wait(&elevator_move);
    int direction, steps;
    int distance = destination - source;
    if (distance > 0) {
        direction = 1;
        steps = distance;
    } else {
        direction = -1;
        steps = -1*distance;
    }
    for (; steps > 0; steps--)
        move_direction(elevator, direction);
    //sem_post(&elevator_move);
}


void elevator_ready(int elevator, int at_floor,
                    void (*move_direction)(int, int), void (*door_open)(int),
                    void (*door_close)(int))
{
    int passenger_id = remove_from_queue();
    if (passenger_id < 0) return;
    // wait for passenger to press button and move
    //while (!passenger_ready_for_pickup);
    //sem_wait(&passenger_ready_for_pickup);
    struct passenger *p = &Passengers[passenger_id];
    p->elevator_id = elevator;

    move_elevator(elevator, at_floor, p->from_floor, move_direction);

    // open door and signal passenger
    door_open(elevator);
    //elevator_at_pickup = 1;
    sem_post(&p->elevator_at_pickup);

    // wait for passenger to enter then close and move
    //while (!passenger_in_elevator);
    sem_wait(&p->passenger_in_elevator);
    door_close(elevator);
    move_elevator(elevator, p->from_floor, p->to_floor, move_direction);

    // open door the signal
    door_open(elevator);
    //elevator_at_destination = 1;
    sem_post(&p->elevator_at_destination);

    // wait fr passenger to leave and close door
    //while (!passenger_exited_elevator);
    sem_wait(&p->passenger_exited_elevator);
    door_close(elevator);
}
