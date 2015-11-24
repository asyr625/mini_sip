#ifndef ROLLOVER_PRONE_TO_MONOTONIC_H
#define ROLLOVER_PRONE_TO_MONOTONIC_H

template <class Rollover_Prone, class Monotonic>
class Rollover_Prone_To_Monotonic
{
public:
    Rollover_Prone_To_Monotonic();
    Monotonic monotonic(const Rollover_Prone &rolloverProne);

private:
    typedef enum { low=0, medium=1, high=2, count=3 } Value;

    bool value_present[count];
    Monotonic rollover_addition;
    unsigned int rollover_addition_count;
};

#endif // ROLLOVER_PRONE_TO_MONOTONIC_H
