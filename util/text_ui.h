#ifndef TEXT_UI_H
#define TEXT_UI_H

#include <list>
#include <vector>

#include "dbg.h"
#include "mini_list.h"
#include "command_string.h"

class Text_UI_Completion_Callback
{
public:
    virtual ~Text_UI_Completion_Callback() {}
    virtual Mini_List<std::string> textui_completion_suggestion(std::string match) = 0;
};


struct completion_cb_item
{
    std::string match;
    Text_UI_Completion_Callback *callback;
};

class Question_Dialog : public SObject
{
public:
    Question_Dialog():nAnswered(0){}
    std::string questionId;
    std::string questionId2;
    std::vector<std::string> questions;
    std::vector<std::string> answers;
    int nAnswered;
};

class Text_UI : Dbg_Handler
{
public:
    static const int plain;
    static const int bold;
    static const int red;
    static const int blue;
    static const int green;

    static const int KEY_LEFT;
    static const int KEY_RIGHT;
    static const int KEY_UP;
    static const int KEY_DOWN;


    Text_UI();
    virtual ~Text_UI();

    void show_question_dialog(const SRef<Question_Dialog*> &d);

    virtual void gui_execute(std::string cmd) = 0;
    virtual void gui_execute(const SRef<Question_Dialog*> &questionAnswer) = 0;

    virtual void display_message(std::string msg, int style=-1);
    virtual void guimain();

    void set_prompt(std::string p);
    void add_command(std::string cmd);

    void add_completion_callback(std::string m, Text_UI_Completion_Callback *cb);
protected:
    int max_hints;
    virtual void key_pressed(int ) {}
    bool running;

private:
    void ask_question();
    void answer_question(std::string answer);
    int make_stdin_nonblocking();

    void restore_stdin_blocking();
    void *terminal_saved_state;

    void output_suggestions(Mini_List<std::string> &strings );
    std::string display_suggestions(std::string hint);

    std::list<SRef<Question_Dialog*> > question_dialogs;
    bool is_asking_dialog_question;

    std::string saved_prompt_text;
    std::string saved_input;

    std::string input;

    int prompt_format;

    std::string prompt_text;

    Mini_List<std::string> commands;

    Mini_List<struct completion_cb_item> completion_callbacks;
};

#endif // TEXT_UI_H
