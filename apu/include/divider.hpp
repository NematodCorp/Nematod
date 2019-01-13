#pragma once

struct divider {
	void tick() {--curr_t; if(curr_t < 0) {curr_t = period; flag = true;} else {flag = false;}};
	bool operator() {return flag;}
	void reset() {curr_t = period; flag = false;}
	
	int period;
protected:
	int curr_t;
	bool flag = false;
};