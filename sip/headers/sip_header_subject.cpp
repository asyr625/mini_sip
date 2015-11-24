#include "sip_header_subject.h"


SRef<Sip_Header_Value *> subjectFactory(const std::string &build_from)
{
    return new Sip_Header_Value_Subject(build_from);
}

Sip_Header_Factory_Func_Ptr sip_header_subject_factory = subjectFactory;

const std::string sipHeaderValueSubjectTypeStr = "Subject";

Sip_Header_Value_Subject::Sip_Header_Value_Subject(const std::string &build_from)
    : Sip_Header_Value_String(SIP_HEADER_TYPE_SUBJECT, sipHeaderValueSubjectTypeStr, build_from)
{
}
