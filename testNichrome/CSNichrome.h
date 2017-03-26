// Header file for CSNichrome
// Stephen Flores
// Implement functions to heat the wire for a specified period of time.

// NOTE: Requires passing the current time to the functions.

// To use:
// Upon transitioning to the release state, call start(currentTime, duration) to
// set the start and duration for the cut. All times are in ms. Start() will
// then set the pin to HIGH, which will start the cut. It is only meant to be
// called once during a state transition, immediately followed by hold().

// The hold(currentTime) function ensures that the pin stays HIGH while
// currentTime < startTime + duration. Once currentTime exceeds this threshold,
// hold() will set the pin to LOW, thus ending the cutting process.

// The end() function is just there cuz I think it should be there. It will
// set the pin to LOW, and set duration to 0, so if hold() is called again,
// then it will immeadiately fall to the else case.

#ifndef _CSNichrome_h
#define _CSNichrome_h

class CSNichrome {
public:
    void init(int p);
    void start(long st, int dur);
    int hold(long t);
    void end();
private:
    int pin;
    int duration;
    long startTime;
};


#endif
