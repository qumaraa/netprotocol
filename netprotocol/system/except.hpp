/*=============================================================================
                    Copyright (c) by QMV Corporation
 * Author: ynwqmv
 * File: except.hpp
 * Description: Exception Classes for Network

 * Cross-platform network written by using Boost.Asio
 * GitHub: https://github.com/ynwqmv/netprotocol
 *===========================================================================*/


#ifndef SYSTEM_EXCEPT_HPP
#define SYSTEM_EXCEPT_HPP

/*
* @netv: here is defined exception classes
*/
namespace netv
{

    /*
    * @system_error: system-category except. class
    */

    class system_error
    {
    public:
        system_error() = default;

        system_error(const std::string& msg) : message(msg) { }

        virtual const char* what() const noexcept { return message.c_str(); }
      

    private:
        std::string message;
    };
    /*
    * @stream_error extends from @system_error: stream-category except. class (fstream, sstream, ostream...)
    */
    class stream_error : public system_error
    { 
        std::string message;

    public:
        stream_error(const std::string& msg) : message(msg) { }
        
        const char* what() const noexcept override { return message.c_str(); }
    };


} // NAMESPACE SYSTEM

#endif // SYSTEM_EXCEPT_HPP
