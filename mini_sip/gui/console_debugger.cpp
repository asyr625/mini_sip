#include <iostream>
using namespace std;

#include "console_debugger.h"
#include "mini_list.h"
#include "string_utils.h"

#include "termmanip.h"
#include "my_error.h"


#ifdef SM_DEBUG
#include "state_machine.h"
#endif

#include "sip_command_string.h"
#include "sip_smcommand.h"
#include "media_command_string.h"

#	include<unistd.h>

#ifdef HAVE_UNISTD_H
#	include<unistd.h>
#endif

#ifdef HAVE_TERMIOS_H
#	include<termios.h>
#endif

#ifdef WIN32
#	include<conio.h>
#	ifdef _WIN32_WCE
#		include<stdio.h>
#	endif
#endif

#ifdef _WIN32_WCE
#	include "minisip_wce_extra_includes.h"
#endif


int globalBitRate1 = 2000;
int globalBitRate2 = 2000;

Console_Debugger::Console_Debugger(SRef<Sip_Configuration*> conf)
    : media_handler(NULL),
      thread( NULL ),
      config( conf )
{
    this->sip_stack = conf->_sip_stack;
    globalBitRate1 = conf->video_encoder1.bitrate;
    globalBitRate2 = conf->video_encoder2.bitrate;
}

Console_Debugger::~Console_Debugger()
{
}

void Console_Debugger::show_help()
{
    cerr << "Welcome to the Console Debugger!" << endl;
    cerr << "   Available commands are:" << endl;
    cerr << "        - h/H : display this help message" << endl;
    cerr << endl;
    cerr << "        - r/R : register all identities" << endl;
    cerr << "        - u/U : de-register all identities" << endl;
    cerr << "        - t/T : terminate all ongoing calls" << endl;
    cerr << endl;
    cerr << "        - d/D : Turn on of the my_dbg output stream (less verbosity)" << endl;
    cerr << "        - p/P : Print IN and OUT packets on the screen" << endl;
    cerr << endl;
    cerr << "        - a/A : List all thread ids currently active" << endl;

    cerr << "        - ; : More memory related debug output (object created or destroyed)" << endl;
    cerr << endl;
    cerr << "        - ( : Output StateMachine related debug messages (see state_machine.h)" << endl;
    cerr << endl;
    cerr << "        - ) : Print a list of timers waiting to be fired" << endl;
    cerr << "        - + : Print info on all currently registered SipDialogs" << endl;
    cerr << endl;
    cerr << "        - m/M : Print info on MediaSessions currently running" << endl;
    cerr << endl;
    cerr << "        - c/C : Print info on internal configuration state" << endl;
    cerr << endl;
    cerr << "   Note: only h/+/u/r/t commands are always available. The rest, only if --enable-debug is configured" << endl;
    cerr << endl;
}

void Console_Debugger::run()
{
#ifdef DEBUG_OUTPUT
    bool tmpSet;
    set_thread_name("Console_Debugger");
#endif

    keep_running = true;
    while(keep_running)
    {
        char c;
#ifdef _MSC_VER
        int n=1;
#	ifdef _WIN32_WCE
        c= getchar();
#	else
        c= _getch();
#	endif
#else
        int n = read(STDIN_FILENO, &c, 1);
#endif
        if( !keep_running )
        {
            //cerr << "CDbg: run(): do not keep running" << endl;
            break;
        }
        if (n==1)
        {
            switch (c)
            {
            case ' ':
            case '\n':
                cerr << endl;
                break;
            case '+'://show sip stack state
                show_stat();
                break;
            case 'c': //print mediahandler session info
            case 'C':
                show_config();
                break;
            case 'h':
            case 'H': //list help
                show_help();
                break;
            case 't':
            case 'T': //Terminate all ongoing calls
                send_management_command( Sip_Command_String::terminate_all_calls );
                break;
            case 'u':
            case 'U': //Deregister all identities
                send_management_command( Sip_Command_String::unregister_all_identities );
                break;
            case 'r':
            case 'R': //Register all identities
                send_management_command( Sip_Command_String::register_all_identities );
                break;

    #ifdef DEBUG_OUTPUT
            case '1':
                globalBitRate1-=250;
                cerr <<"------->EEEE: bitrate1="<<globalBitRate1<<endl;
                break;
            case '2':
                globalBitRate1+=250;
                cerr <<"------->EEEE: bitrate1="<<globalBitRate1<<endl;
                break;
            case '3':
                globalBitRate2-=250;
                cerr <<"------->EEEE: bitrate2="<<globalBitRate2<<endl;
                break;
            case '4':
                globalBitRate2+=250;
                cerr <<"------->EEEE: bitrate2="<<globalBitRate2<<endl;
                break;
            case 'P':
            case 'p':
                sip_stack->set_debug_print_packets(!sipStack->get_debug_print_packets() );
                if (sip_stack->get_debug_print_packets() )
                    cerr << "Packets will be displayed to the screen"<< endl;
                else
                    cerr << "Packets will NOT be displayed to the screen"<< endl;
                break;

            case 'd':
            case 'D':
                my_dbg.set_enabled( ! my_dbg.get_enabled() );
                if (my_dbg.get_enabled()){
                    cerr << "Debug information ON"<< endl;
                }else{
                    cerr << "Debug information OFF"<< endl;
                }
                break;

            case 'a':
            case 'A':
                print_threads();
                break;
            case '<':
            case '*':
                show_mem();
                break;

            case '>':
                show_mem_summary();
                break;

            case ';': //output message when object is destroyed
                tmpSet = set_debug_output(true);
                if( tmpSet )
                    cerr << "MemObject debug info turned ON"<< endl;
                else
                    cerr << "MemObject debug info turned OFF"<< endl;
                break;

            case '(': //turn on/off state machine debug
        #ifdef SM_DEBUG
                outputStateMachineDebug = !outputStateMachineDebug;
                if( outputStateMachineDebug )
                    cerr << "StateMachine debug info turned ON"<< endl;
                else
                    cerr << "StateMachine debug info turned OFF"<< endl;
        #else
                cerr << "StateMachine debug not usable: need to #define SM_DEBUG (see StateMachine.h)"<< endl;
        #endif
                break;
            case ')': //print all timers ...
                cerr << "========= Timeouts still to fire : " << endl;
                cerr << sip_stack->get_timeout_provider()->get_timeouts() << endl;
                cerr << "=========------------------ " << endl;
                break;

            case 'm': //print mediahandler session info
            case 'M':
                send_command_to_media_handler(Media_Command_String::session_debug);
                break;

    #endif
            default:
                cerr << "Unknown command: "<< c << endl;
            }
        }

    }
}

void Console_Debugger::send_management_command( std::string str )
{
    Command_String cmdstr ( "", str );
    Sip_SMCommand cmd( cmdstr, Sip_SMCommand::dialog_layer, Sip_SMCommand::dispatcher);
    sip_stack->handle_command(cmd);
}

void Console_Debugger::send_command_to_media_handler( std::string str )
{
    Command_String cmdstr ("", str);
    std::cerr << "========= Media_Handler Debug info : " << std::endl;
    if (media_handler)
        media_handler->handle_command("media", cmdstr );
    else
        std::cerr << "(no media handler registred)"<< std::endl;

    std::cerr << "=========" << std::endl;
}

void Console_Debugger::show_mem()
{
    std::string all;
    Mini_List<std::string> names = get_mem_object_names();
    for (int i = 0; i < names.size();i++)
    {
        all = all + names[i] + "\n";
    }
    std::cerr << all << itoa(get_mem_object_count()) <<" objects"<< std::endl;
}

void Console_Debugger::show_mem_summary()
{
    std::string all;
    Mini_List<std::string> names = get_mem_object_names_summary();
    int i;
    for (i = 0; i < names.size();i++)
    {
        all = all + names[i] + "\n";
    }
    std::cerr << all << i <<" types of objects"<< std::endl;
}

void Console_Debugger::show_stat()
{
    my_out << sip_stack->get_stack_status_debug_string();
}

void Console_Debugger::show_config()
{
    cerr <<    "SipSoftPhoneConfiguration:"<<endl
        << "  Minisip:" << endl
        << "    useSTUN="<<config->_use_stun<<endl
        << "    stunServerIpString="<<config->_stun_server_ip_string<<endl
        << "    stunServerPort="<<config->_stun_server_port<<endl
        << "    findStunServerFromSipUri="<<config->_find_stun_server_from_sip_uri<<endl
        << "    findStunServerFromDomain="<<config->_find_stun_server_from_domain<<endl
        << "    stunDomain="<<config->_stun_domain<<endl
        << "    useUserDefinedStunServer="<< config->_use_user_defined_stun_server <<endl
        << "    userDefinedStunServer=" << config->_user_defined_stun_server << endl
        << "    soundDeviceIn="<< config->_sound_device_in  <<endl
        << "    soundDeviceOut="<< config->_sound_device_out<< endl
        << "    videoDevice="<< config->_video_device << endl
        << "    frameWidth=" << config->_frame_width << endl
        << "    frameHeight="<< config->_frame_height << endl
        << "    usePSTNProxy="<< config->_use_pstn_proxy <<endl
// 		<< "    tcp_server="<< config->tcp_server<<  endl
// 		<< "    tls_server="<< config->tls_server << endl
        << "    ringtone="<< config->_ringtone << endl
        << "    soundIOmixerType="<<config->_sound_iomixer_type <<endl
        << "    networkInterfaceName="<< config->_network_interface_name <<endl
        << "  SipStackConfig:"<<endl
        << "    localIpString="<<config->_sip_stack_config->local_ip_string<<endl
        << "    localIp6String="<< config->_sip_stack_config->local_ip6_string << endl
        << "    externalContactIP=" << config->_sip_stack_config->external_contact_ip << endl
        << "    externalContactUdpPort=" << config->_sip_stack_config->external_contact_udp_port << endl
// 		<< "    preferedLocalUdpPort="<< config->_sip_stack_config->preferedLocalUdpPort << endl //TODO: output actual port in use as well?
// 		<< "    preferedLocalTcpPort="<< config->_sip_stack_config->preferedLocalTcpPort << endl
// 		<< "    preferedLocalTlsPort="<< config->_sip_stack_config->preferedLocalTlsPort << endl
        << "    autoAnswer="<< config->_sip_stack_config->auto_answer<<endl
        << "    use100Rel="<< config->_sip_stack_config->use_100_rel<< endl
        << "    instanceId="<< config->_sip_stack_config->instance_id<<endl
        << "    Certificates:"<<endl;

    if (config->_sip_stack_config->cert)
        config->_sip_stack_config->cert->lock();
    if (config->_sip_stack_config->cert && config->_sip_stack_config->cert->length()>0)
    {
        int n = 1;
        SRef<Certificate *> crt=config->_sip_stack_config->cert->get_first();
        while (crt)
        {
            cerr << "      certificate "<<n<<endl
                 << "        name="<<crt->get_name()<<endl
                 << "        cn="<<crt->get_cn()<<endl
                 << "        issuer="<<crt->get_issuer()<<endl
                 << "        issuer_cn="<< crt->get_issuer_cn()<<endl
                 << "        has_pk="<< crt->has_pk()<<endl
                 << "        SubjectAltName,"<< endl;
#if 0
//Print all subjectAltName here - FIXME
            //Note: if the enum declaration in cert.h changes
            //we get a bug here.
            char *types[]={0,"SAN_DNSNAME", "SAN_RFC822NAME", "SAN_URI", "SAN_IPADDRESS",0};
            certificate::SubjectAltName san= certificate::SAN_DNSNAME;

            // there are four alt name types, and each can be
            // multiple values.
            for (int i=1; types[i]; i++)
            {
                cerr << "          type "<< types[i]<<": ";
                vector<string> alt = crt->get_alt_name(san);
                vector<string>::iterator j;
                int n=0;
                for (j=alt.begin(); j!=alt.end(); j++,n++){
                    if (n){
                        cerr << ", "; // do not output before first element
                    }
                    cerr << *j;
                }
                cerr << endl;
                san=san+1;
            }
#endif
            crt = config->_sip_stack_config->cert->get_next();
            n++;
        }
    }else{
        cerr    <<      "        (no certificate)"<<endl;
    }
    if (config->_sip_stack_config->cert)
        config->_sip_stack_config->cert->unlock();
}

static int nonblockin_stdin()
{
#ifdef HAVE_TERMIOS_H
    struct termios termattr;
    int ret=tcgetattr(STDIN_FILENO, &termattr);
    if (ret < 0) {
        my_error("tcgetattr:");
        return -1;
    }
    termattr.c_cc[VMIN]=1;
    termattr.c_cc[VTIME]=0;
    termattr.c_lflag &= ~(ICANON | ECHO | ECHONL);

    ret = tcsetattr (STDIN_FILENO, TCSANOW, &termattr);
    if (ret < 0) {
        my_error("tcsetattr");
        return -1;
    }
#endif
    return 0;
}


SRef<Thread *> Console_Debugger::start()
{
    nonblockin_stdin();

    thread = new Thread(this, Thread::High_Priority);
    return thread;
}

void Console_Debugger::stop()
{
    keep_running = false;
    thread->kill();
}

void Console_Debugger::join()
{
    thread->join();
    media_handler = NULL;
    sip_stack = NULL;
    thread = NULL;
}
