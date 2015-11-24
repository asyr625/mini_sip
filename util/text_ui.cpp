#include "util_defines.h"
#include "text_ui.h"

#include<stdio.h>

#ifdef HAVE_TERMIOS_H
#include<termios.h>
#endif

#if defined WIN32 || defined _MSC_VER
#include<conio.h>
#include<windows.h>
#endif

#ifdef _MSC_VER
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include<io.h>
#else
#include<unistd.h>
#endif

#include "my_error.h"
#include "string_utils.h"
#include "my_assert.h"

#include<iostream>

using namespace std;

const int Text_UI::plain = 0;
const int Text_UI::bold = 1;
const int Text_UI::red = 2;
const int Text_UI::blue = 3;
const int Text_UI::green = 4;

const int Text_UI::KEY_UP = 0x5b41;
const int Text_UI::KEY_DOWN = 0x5b42;
const int Text_UI::KEY_RIGHT = 0x5b43;
const int Text_UI::KEY_LEFT = 0x5b44;

#if defined(_MSC_VER) || defined(WIN32)
const char *termCodes[]= { "", "", "", "", "" };
#else
const char *termCodes[]= { "\033[m", "\033[2m\033[1m", "\033[31m", "\033[34m", "\033[42m" };
#endif

/**
 * Purpose: Tries to make STDIN non-blocking.
 *
 * This method stores the current state of the terminal to an internal
 * structure that is used to restore the state when restoreStdinBlocking()
 * is called.
 *
 * This method is not implementedMicrosoft Windows.
 *
 * @return 	0 	If successful
 * 		!=0	If the terminal/stdin could not be set to
 * 			nonblocking.
 */
int Text_UI::make_stdin_nonblocking()
{
#ifdef HAVE_TERMIOS_H
    my_assert(terminal_saved_state);
    tcgetattr(STDIN_FILENO, (struct termios*)terminal_saved_state);

    struct termios termattr;
    int ret = tcgetattr(STDIN_FILENO, &termattr );
    if (ret < 0)
    {
        delete (struct termios*)terminal_saved_state;
        terminal_saved_state = NULL;
        my_error("tcgetattr:");
        return -1;
    }
    termattr.c_cc[VMIN]=1;
    termattr.c_cc[VTIME]=0;
    termattr.c_lflag &= ~(ICANON | ECHO | ECHONL);

    ret = tcsetattr (STDIN_FILENO, TCSANOW, &termattr);
    if (ret < 0)
    {
        my_error("tcsetattr");
        return -1;
    }
    return 0;
#else
#if defined(WIN32) || defined(_MSC_VER)
    HANDLE console = GetStdHandle(STD_INPUT_HANDLE);
    if (!console)
    {
        my_err << "Error: could not get input handle"<<endl;
        return -1;
    }
    DWORD oldMode;
    if (!GetConsoleMode(console, &oldMode))
    {
        my_err << "Error: could not get console mode"<<endl;
        return -1;
    }
    DWORD mode = oldMode & ~ENABLE_LINE_INPUT & ~ENABLE_ECHO_INPUT;
    if (!SetConsoleMode(console, mode))
    {
        cerr << "Error: could not set console mode"<<endl;
        return -1;
    }
    return 0;
#endif
#endif
}

/**
 * Restores the terminal state to the one previous to running
 * makeStdinNonblocking.
 * Precondition: makeStdinNonblocking has been run.
 */
void Text_UI::restore_stdin_blocking()
{
#ifdef HAVE_TERMIOS_H
    if (terminal_saved_state)
    {
        int ret = tcsetattr (STDIN_FILENO, TCSANOW, (struct termios*)terminal_saved_state);
        if (ret < 0)
        {
            my_error("tcsetattr");
        }
    }
#else
#if defined(WIN32) || defined(_MSC_VER)
    HANDLE console = GetStdHandle(STD_INPUT_HANDLE);
    if (!console)
    {
        cerr << "Error: could get console handle"<<endl;
        return;
    }
    DWORD oldMode;
    if (!GetConsoleMode(console, &oldMode))
    {
        cerr << "Error: could get console mode"<<endl;
        return;
    }
    DWORD mode = oldMode | ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT;
    if (!SetConsoleMode(console, mode))
    {
        cerr << "Error: could not restore console mode"<<endl;
    }
#endif
#endif
}

Text_UI::Text_UI()
    : max_hints(2000),is_asking_dialog_question(false)
{
#ifdef HAVE_TERMIOS_H
    terminal_saved_state = new struct termios;
#else
    terminal_saved_state = NULL;
#endif

    if ( make_stdin_nonblocking() != 0)
    {
        cerr << "ERROR: Could not make stdin non-blocking"<< endl;
        //        exit(1);
    }
    running = true;
}

Text_UI::~Text_UI()
{
    restore_stdin_blocking();
#ifdef HAVE_TERMIOS_H
    if (terminal_saved_state)
    {
        delete (struct termios*)terminal_saved_state;
    }
#endif
}

void Text_UI::show_question_dialog(const SRef<Question_Dialog*> &d)
{
    question_dialogs.push_back(d);
    if (!is_asking_dialog_question )
    {
        ask_question();
    }
}

void Text_UI::ask_question()
{
    if (question_dialogs.size()<=0)
    {
        return;
    }

    if (!is_asking_dialog_question)
    {
        is_asking_dialog_question = true;
        saved_prompt_text = prompt_text;
        saved_input = input;
    }
    SRef<Question_Dialog*> dialog = question_dialogs.front();
    string q = dialog->questions[dialog->nAnswered];
    prompt_text = "Question: "+q;
    input= "";
    display_message("");
}

void Text_UI::answer_question(std::string answer)
{
    SRef<Question_Dialog *> d = question_dialogs.front();
    d->answers.push_back(answer);
    d->nAnswered++;
    if (d->nAnswered >= (int)d->questions.size())
    {
        //all questions now answered
        gui_execute(d);
        is_asking_dialog_question = false;
        question_dialogs.pop_front();
        input = saved_input;
        prompt_text = saved_prompt_text;
        display_message("");
    }
    ask_question();
}

void Text_UI::output_suggestions( Mini_List<std::string> &strings )
{
    string out;
    for (int i=0; i<max_hints && i< strings.size(); i++)
    {
        if (i!=0)
            out+=" | ";
        out+= strings[i];
    }
    display_message(out);
}

std::string Text_UI::display_suggestions( std::string hint)
{
    Mini_List<string> cbSuggest;
    int i;
    for (i=0; i< completion_callbacks.size();i++)
    {
        Mini_List<string> tmp;
        if (completion_callbacks[i].match == trim(hint).substr(0,completion_callbacks[i].match.size()))
        {
            tmp = completion_callbacks[i].callback->textui_completion_suggestion(hint);
            for (int j=0; j<tmp.size(); j++)
            {
                if (hint=="" || (tmp[j].size() >= hint.size() && tmp[j].substr(0, hint.size()) == hint ))
                {
                    cbSuggest.push_back(tmp[j]);
                }
            }
        }
    }

    int ncmds = commands.size();
    if (ncmds + cbSuggest.size() <= 0)
        return "";

    Mini_List<string> res;

    int nfound = 0;
    for (i=0; i<ncmds; i++) //Find possible matches
    {
        if (hint=="" || (commands[i].size()>=hint.size() && commands[i].substr(0,hint.size())==hint ))
        {
            nfound++;
            res.push_back(commands[i]);
        }
    }

    if (cbSuggest.size()==0)
    {
        for (i=0; i<completion_callbacks.size(); i++)
        {
            if (hint=="" || (completion_callbacks[i].match.size()>=hint.size() && completion_callbacks[i].match.substr(0,hint.size())==hint ))
            {
                nfound++;
                res.push_back(completion_callbacks[i].match);
            }
        }
    }
    if (nfound+cbSuggest.size()==0)
        return "";

    if (nfound==1 && cbSuggest.size()==0)
        return res[0]+" ";

    if (nfound==0 && cbSuggest.size()==1)
        return cbSuggest[0]+" ";


    for (i=0; i<res.size(); i++)
        cbSuggest.push_back(res[i]);

    //Now all suggested strings are in cbSuggest (the ones from the
    //callback plus the ones matching from "command")

    bool cont = true;
    unsigned index = (unsigned)hint.size();
    while (cont)
    {
        char c=0;
        for (i=0; i< cbSuggest.size(); i++)
        {
            if (cbSuggest[i].size()<=index)
                cont = false;
            else
            {
                if (!c)
                {
                    c = cbSuggest[i][index];
                }
                else
                {
                    if (cbSuggest[i].size()<=index || cbSuggest[i][index]!=c)
                        cont=false;
                }
            }
        }
        if (cont)
            index++;
    }

    if (cbSuggest.size()>0 && index<=hint.size())
    {
        output_suggestions(cbSuggest);
    }

    if (index>hint.size())
    {
        string ac = cbSuggest[0].substr(0,index);

        if (cbSuggest.size()==1 && ac[ac.size()-1] !=' ')
            ac+=" ";
        return ac;
    }
    else
        return "";
}

void Text_UI::display_message(std::string msg, int style)
{
    cout << (char)13;
    if (msg.size()>0 && msg[msg.size()-1]==10)
        msg=msg.substr(0,msg.size()-1);

    if (style<0)
    {
        cout << msg;
    }
    else
    {
        cout << termCodes[style];
        cout << msg << termCodes[plain];
    }

    if (msg.size()<=input.size()+prompt_text.size()+2)  // 2==strlen("$ ")
    {
        //cout << "will loop for "<< input.size()+prompt.size()-msg.size()+1 << endl;
        //cout << "input.size="<< input.size() << " prompt.size=" << prompt.size()<< endl;
        //cout << "prompt is <"<< prompt << ">"<< endl;
        for (unsigned i=0; i< input.size()+prompt_text.size()+2-msg.size(); i++)
        {
            cout << ' ';
        }
    }

    cout << endl;
    cout << prompt_text<<"$ ";
    cout << termCodes[bold]<< input << termCodes[plain] << flush;
}

void Text_UI::set_prompt(std::string p)
{
    prompt_text = p;
}

void Text_UI::add_command(std::string cmd)
{
    commands.push_back(cmd);
}

void Text_UI::add_completion_callback(std::string m, Text_UI_Completion_Callback *cb)
{
    struct completion_cb_item cbi;
    cbi.match = m;
    cbi.callback = cb;
    completion_callbacks.push_back(cbi);
}


int read_char()
{
    int err;
    int ret;
    char tmpc;
#if defined WIN32 || defined _MSC_VER
    //	tmpc = getchar();
    int n;
    if (!ReadConsole(GetStdHandle(STD_INPUT_HANDLE),&tmpc,1,(LPDWORD)&n, NULL)){
        cerr << "Console Error: error reading from stdin"<<endl;
    }
    err=1;
#else
    err = read(STDIN_FILENO, &tmpc, 1);
#endif
    ret = tmpc;

    if (err != 1)
        return -1;

    //handle special keys prefixed with escape
    if (tmpc == 27)
    {
        char c1,c2;
#if defined WIN32 || defined _MSC_VER
        //c1 = getchar();
        ReadConsole(GetStdHandle(STD_INPUT_HANDLE),&c1,1,(LPDWORD)&n, NULL);
        //c2 = getchar();
        ReadConsole(GetStdHandle(STD_INPUT_HANDLE),&c2,1,(LPDWORD)&n, NULL);
        err=1;
#else
        err = read(STDIN_FILENO, &c1, 1);
        err = read(STDIN_FILENO, &c2, 1);
        if (err != 1)
            return -1;
#endif
        ret = c1<<8 | c2;
    }

    return ret;
}

void Text_UI::guimain()
{
    cout << prompt_text << "$ "<< flush;
    while (running)
    {
        int c = read_char();
        if(c >= 0)
        {
            key_pressed(c);

            string command;
            string autocomplete;
            unsigned i;

            switch(c)
            {
            case 9:
                autocomplete = display_suggestions(input);
                if (autocomplete.size()>0)
                    input=autocomplete;

                cout << (char)13<<prompt_text<<"$ "<< termCodes[bold]<< input <<termCodes[plain] << flush;
                break;
            case 10:
            case 13:
                command = trim(input);
                if (command.size()>0)
                    cout <<endl;
                input="";

                if (is_asking_dialog_question)
                {
                    answer_question(command);
                    break;
                }

                if (command.size()==0)
                {
                    display_message("");
                }else{
                    gui_execute(command);
                }

            case 8:
            case 127:
                input = input.substr(0, input.size()-1);
                cout << (char)13<<prompt_text<< "$ ";

                for (i=0; i< input.size()+1; i++)
                    cout << ' ';
                cout << (char)13<<prompt_text<<"$ ";
                cout << termCodes[bold]<< input << termCodes[plain] << flush;
                break;

            default:
                //     @A-Z               a-z                  . / 0-9 :           space     .
                if ( (c>=64 && c<=90) || (c>=97 && c<=122) || (c>=46 && c<=58) || c==32  || c=='-' || c=='_' || c=='.' || c=='\\' || c=='=' )
                {
                    input+=(char)c;
                    cout << termCodes[bold]<< (char)c << termCodes[plain] << flush;
                }
            }
        }
        //		else sleep(1);
    }
}
