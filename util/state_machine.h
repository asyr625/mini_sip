#ifndef STATE_MACHINE_H
#define STATE_MACHINE_H
#include <list>

#include "sobject.h"
#include "timeout_provider.h"

template<class Command_Type, class Timeout_Type>
class State_Transition;

template<class Command_Type, class Timeout_Type>
class State;


template<class Command_Type, class Timeout_Type>
class State_Machine : public virtual SObject
{
public:
    SRef<State<Command_Type, Timeout_Type>* > any_state;

    State_Machine( SRef<Timeout_Provider<Timeout_Type, SRef<State_Machine<Command_Type, Timeout_Type> *>  > *> tp);

    virtual ~State_Machine();

    void free_state_machine();

    std::string get_mem_object_type() const { return "StateMachine"; }

    void add_state(SRef<State<Command_Type, Timeout_Type>* > state);

    SRef<State<Command_Type, Timeout_Type>* > get_state(const std::string &name);

    void set_current_state(SRef<State<Command_Type, Timeout_Type>* > state);

    std::string get_current_state_name() const;

    virtual bool handle_command(const Command_Type& command);

    //int
    void request_timeout(int ms, const Timeout_Type& command);

    void cancel_timeout(const Timeout_Type& command);

    int get_timeout(const Timeout_Type& command);

    SRef<Timeout_Provider<Timeout_Type, SRef<State_Machine<Command_Type, Timeout_Type> *> > *> get_timeout_provider();

    virtual void handle_timeout(const Timeout_Type&);

    void timeout(const Timeout_Type& command);

private:
    std::list<SRef<State<Command_Type, Timeout_Type>* > > _states;
    SRef<State<Command_Type, Timeout_Type>* >   _current_state;
    SRef<Timeout_Provider<Timeout_Type, SRef<State_Machine<Command_Type, Timeout_Type> *> > *> _timeout_provider;
};


template<class Command_Type, class Timeout_Type>
class State : public SObject
{
public:
    State(SRef<State_Machine<Command_Type,Timeout_Type> *> state_machine,
          const std::string &name);
    ~State();

    void free_state();

    std::string get_mem_object_type() const { return "State"; }

    void register_transition(SRef<State_Transition<Command_Type, Timeout_Type> *> transition);

    SRef<State_Transition<Command_Type, Timeout_Type>* > get_transition(const std::string &name);

    bool remove_transition(const std::string & name);
    bool handle_command(const Command_Type& command);
    std::string get_name();
private:
    SRef<State_Machine<Command_Type, Timeout_Type>* >   _state_machine;
    std::string _name;
    std::list<SRef<State_Transition<Command_Type, Timeout_Type>* > >    _transitions;
};

template<class Command_Type, class Timeout_Type>
class State_Transition : public SObject
{
public:
    State_Transition(SRef<State_Machine<Command_Type, Timeout_Type>* > state_machine,
                     const std::string &name,
                     bool (State_Machine<Command_Type, Timeout_Type>::*a)(const Command_Type&),
                     SRef<State<Command_Type, Timeout_Type> *> from_state,
                     SRef<State<Command_Type, Timeout_Type> *> to_state);
    ~State_Transition();
    std::string get_mem_object_type() const { return "StateTransition"; }
    bool handle_command(const Command_Type& c);
    std::string get_name();
private:
    SRef<State_Machine<Command_Type, Timeout_Type>* >   _state_machine;
    std::string _name;

    bool (State_Machine<Command_Type, Timeout_Type>::*_action)(const Command_Type&);

    SRef<State<Command_Type, Timeout_Type> *>   _from_state;
    SRef<State<Command_Type, Timeout_Type> *>   _to_state;
};

#endif // STATE_MACHINE_H
