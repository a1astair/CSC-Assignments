This program simulates a priority queueing system (PQS). In such a system, there are arriving customers and
a clerk who serves the customers. When a customer arrives and the clerk is busy with serving other customers, the
arriving customer needs to wait. It assumes a work-conserving system, i.e., the clerk cannot be idle if there are
customers waiting for service. Each customer is given a priority number, it assumes customers
are given before hand and their description is stored in a file.

This program uses threads to simulates the customers and schedules these threads appropiately using condition variables and mutexes