#ifndef __STRING_HPP__
#define __STRING_HPP__

#include <ms/impl/common.hpp>
#include <ms/impl/list.hpp>

MS_STD_BEGIN

using strsize_t = u16;

class CString : public CList<char, strsize_t> {
public:

};

MS_STD_END


#endif