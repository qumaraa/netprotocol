/* SYSTEM/EXCEPT.HPP * exception classes | v3.2.9 */


#ifndef SYSTEM_EXCEPT_HPP
#define SYSTEM_EXCEPT_HPP


namespace netv
{
    class system_error
    {
    public:
        system_error() = default;

        system_error(const std::string& msg) : message(msg) { }

        virtual const char* what() const noexcept { return message.c_str(); }

    private:
        std::string message;
    };

    class stream_error : public system_error
    { 
        std::string message;

    public:
        stream_error(const std::string& msg) : message(msg) { }
        
        const char* what() const noexcept override { return message.c_str(); }
    };


} // NAMESPACE SYSTEM

#endif // SYSTEM_EXCEPT_HPP
