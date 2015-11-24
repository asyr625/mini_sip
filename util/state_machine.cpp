#include "state_machine.h"

template<class Command_Type, class Timeout_Type>
State_Machine<Command_Type,Timeout_Type>::State_Machine( SRef<Timeout_Provider<Timeout_Type, SRef<State_Machine<Command_Type, Timeout_Type> *>  > *> tp)
    : _current_state(NULL), _timeout_provider(tp)
{
    any_state = new State<Command_Type,Timeout_Type>(this,"libutil_any");
}

template<class Command_Type, class Timeout_Type>
State_Machine<Command_Type,Timeout_Type>::~State_Machine()
{

}

template<class Command_Type, class Timeout_Type>
void State_Machine<Command_Type,Timeout_Type>::free_state_machine()
{
    _current_state = NULL;
    _timeout_provider = NULL;
    any_state->free_state();
    any_state = NULL;

    typename std::list<SRef<State<Command_Type,Timeout_Type> *> >::iterator iter;
    typename std::list<SRef<State<Command_Type,Timeout_Type> *> >::iterator iter_end = _states.end();
    for( iter = _states.begin(); iter != iter_end; ++ iter )
    {
        (*iter)->free_state();
    }
    _states.clear();
}


template<class Command_Type, class Timeout_Type>
void State_Machine<Command_Type,Timeout_Type>::add_state(SRef<State<Command_Type, Timeout_Type>* > state)
{
    if( !_current_state )
    {
        _current_state = state;
        _states.push_back(state);
    }
}

template<class Command_Type, class Timeout_Type>
SRef<State<Command_Type, Timeout_Type>* > State_Machine<Command_Type,Timeout_Type>::get_state(const std::string &name)
{
    typename std::list<SRef<State<Command_Type,Timeout_Type> *> >::iterator iter;
    typename std::list<SRef<State<Command_Type,Timeout_Type> *> >::iterator iter_end = _states.end();
    for( iter = _states.begin(); iter != iter_end; ++ iter )
    {
        if( (*iter)->get_name() == name )
            return (*iter);
    }
    return NULL;
}

template<class Command_Type, class Timeout_Type>
void State_Machine<Command_Type,Timeout_Type>::set_current_state(SRef<State<Command_Type, Timeout_Type>* > state)
{
    _current_state = state;
}

template<class Command_Type, class Timeout_Type>
std::string State_Machine<Command_Type,Timeout_Type>::get_current_state_name() const
{
    return _current_state ? _current_state->get_name() : "";
}

template<class Command_Type, class Timeout_Type>
bool State_Machine<Command_Type,Timeout_Type>::handle_command(const Command_Type& command)
{
    if( _current_state )
    {
        return _current_state->handle_command(command) || any_state->handle_command(command);
    }
    else
        return false;
}


template<class Command_Type, class Timeout_Type>
void State_Machine<Command_Type,Timeout_Type>::request_timeout(int ms, const Timeout_Type& command)
{
    _timeout_provider->request_timeout(ms, this, command);
}

template<class Command_Type, class Timeout_Type>
void State_Machine<Command_Type,Timeout_Type>::cancel_timeout(const Timeout_Type& command)
{
    _timeout_provider->cancel_request(this, command);
}

template<class Command_Type, class Timeout_Type>
int State_Machine<Command_Type,Timeout_Type>::get_timeout(const Timeout_Type& command)
{
    return _timeout_provider->get_timeout(this, command);
}

template<class Command_Type, class Timeout_Type>
SRef<Timeout_Provider<Timeout_Type, SRef<State_Machine<Command_Type, Timeout_Type> *> > *> State_Machine<Command_Type,Timeout_Type>::get_timeout_provider()
{
    return _timeout_provider;
}

template<class Command_Type, class Timeout_Type>
void State_Machine<Command_Type,Timeout_Type>::handle_timeout(const Timeout_Type&)
{
#ifdef DEBUG_OUTPUT
    std::cerr << "WARNING: UNIMPLEMENTED handleTimeout" << std::endl;
#endif
}

template<class Command_Type, class Timeout_Type>
void State_Machine<Command_Type,Timeout_Type>::timeout(const Timeout_Type& command)
{
    handle_timeout(command);
}

template<class Command_Type, class Timeout_Type>
State<Command_Type,Timeout_Type>::State(SRef<State_Machine<Command_Type,Timeout_Type> *> state_machine,
                                        const std::string &name)
    : _state_machine(state_machine), _name(name)
{

}

template<class Command_Type, class Timeout_Type>
State<Command_Type,Timeout_Type>::~State()
{
    free_state();
}

template<class Command_Type, class Timeout_Type>
void State<Command_Type,Timeout_Type>::free_state()
{
    _state_machine = NULL;
    _transitions.clear();
}

template<class Command_Type, class Timeout_Type>
void State<Command_Type,Timeout_Type>::register_transition(SRef<State_Transition<Command_Type, Timeout_Type> *> transition)
{
    _transitions.push_back(transition);
}

template<class Command_Type, class Timeout_Type>
SRef<State_Transition<Command_Type, Timeout_Type>* > State<Command_Type,Timeout_Type>::get_transition(const std::string &name)
{
    typename std::list<SRef<State_Transition<Command_Type,Timeout_Type> *> >::iterator iter;
    typename std::list<SRef<State_Transition<Command_Type,Timeout_Type> *> >::iterator iter_end = _transitions.end();
    for( iter = _transitions.begin(); iter != iter_end; ++iter)
    {
        if( (*iter)->get_name() == name )
            return *iter;
    }
    return NULL;
}

template<class Command_Type, class Timeout_Type>
bool State<Command_Type,Timeout_Type>::remove_transition(const std::string & name)
{
    typename std::list<SRef<State_Transition<Command_Type,Timeout_Type> *> >::iterator iter;
    typename std::list<SRef<State_Transition<Command_Type,Timeout_Type> *> >::iterator iter_end = _transitions.end();
    for( iter = _transitions.begin(); iter != iter_end; ++iter)
    {
        if( (*iter)->get_name() == name )
            _transitions.erase(iter);
        return true;
    }
    return false;
}

template<class Command_Type, class Timeout_Type>
bool State<Command_Type,Timeout_Type>::handle_command(const Command_Type& command)
{
    typename std::list<SRef<State_Transition<Command_Type,Timeout_Type> *> >::iterator iter;
    typename std::list<SRef<State_Transition<Command_Type,Timeout_Type> *> >::iterator iter_end = _transitions.end();
    for( iter = _transitions.begin(); iter != iter_end; ++iter)
    {
        if( (*iter)->handle_command(command))
        {
            return true;
        }
    }
    return false;
}

template<class Command_Type, class Timeout_Type>
std::string State<Command_Type,Timeout_Type>::get_name()
{
    return _name;
}


template<class Command_Type, class Timeout_Type>
State_Transition<Command_Type, Timeout_Type>::State_Transition(SRef<State_Machine<Command_Type, Timeout_Type>* > state_machine,
                                                               const std::string &name,
                                                               bool (State_Machine<Command_Type, Timeout_Type>::*a)(const Command_Type&),
                                                               SRef<State<Command_Type, Timeout_Type> *> from_state,
                                                               SRef<State<Command_Type, Timeout_Type> *> to_state)
    : _state_machine(state_machine), _name(name), _action(a),
      _from_state(from_state), _to_state(to_state)
{
    _from_state->register_transition(this);
}

template<class Command_Type, class Timeout_Type>
State_Transition<Command_Type, Timeout_Type>::~State_Transition()
{
    _state_machine = NULL;
    _from_state = NULL;
    _to_state = NULL;
}


template<class Command_Type, class Timeout_Type>
bool State_Transition<Command_Type, Timeout_Type>::handle_command(const Command_Type& c)
{
    bool handled;
    my_assert( _action != (bool (State_Machine<Command_Type,Timeout_Type>::*)(const Command_Type& ))NULL);
    if( ( handled = ((**_state_machine).*_action)(c) ) )
    {
        if( !( _to_state == _state_machine->any_state) )
            _state_machine->set_current_state( _to_state );
#ifdef MSM_DEBUG
        if( outputStateMachineDebug )
        {
            my_err << "MSM_DEBUG: "<< _from_state->get_name()<<" --> "<< _to_state->get_name()<<":\t" <<
                    _state_machine->get_mem_object_type() << ": transition: " << _name;
#ifdef MSM_DEBUG_COMMAND
            my_err << " ("<< c << ")";
#endif
            my_err << std::endl;
        }
#endif
    }
#ifdef MSM_DEBUG
    else if( outputStateMachineDebug )
    {

        //Activate if needed ... it produces quite some extra debug ...
        //merr << "MSM_DEBUG: " << stateMachine->getMemObjectType() << ": Transition Failed: " << name << ": " << from_state->getName()
        //	<<" -> "<<to_state->getName() << end;
    }
#endif

    return handled;
}
template<class Command_Type, class Timeout_Type>
std::string State_Transition<Command_Type, Timeout_Type>::get_name()
{
    return _name;
}
