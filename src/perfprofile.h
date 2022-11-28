/*
 * perfprofile.h
 *
 *  Created on: Sep 26, 2022
 *      Author: Kenny
 */

#ifndef PERFPROFILE_H_
#define PERFPROFILE_H_

#include <sys/time.h>
#include <iostream>

struct timeval tp;
long int current_time_millis(){
	gettimeofday(&tp, NULL);
	return tp.tv_sec * 1000 + tp.tv_usec / 1000;
}

inline long int last_time = 0;

void start_timer(){
	last_time = current_time_millis();
}

void end_timer(){
	using namespace std;

	long int current_time = current_time_millis();

	cout << "latency " << (current_time - last_time) << endl;
}

#endif /* PERFPROFILE_H_ */
