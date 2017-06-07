// Stephen Flores
// CS Buzzer code
// not too much!
// Maybe reduce to analog Writes in order to control duty cycle?

class CSBuzzer {
public:
    CSBuzzer(); // Default
    CSBuzzer(int p);
    void setFrequency(int f);
    void play();
private:
    int pin;
    int frequency;
    bool on;
};
