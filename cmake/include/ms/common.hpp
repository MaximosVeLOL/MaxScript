#ifndef __COMMON_HPP__
#define __COMMON_HPP__

#define MS_BEGIN namespace ms {


#define MS_END }

#ifdef MaxScript_EXPORTS
#define MS_API __declspec(dllexport)
#else
#define MS_API __declspec(dllimport)
#endif

#endif
