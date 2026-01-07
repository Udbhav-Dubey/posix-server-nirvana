#pragma once
#include <string>
class HttpResponse{
    private:
        int status_code;
        std::string content_type;
        std::string body_;
        std::string content_disposition;
    public:
    void SetBody(const std::string &body);
    const std::string build() ;
    static HttpResponse json(const std::string &data);
    static HttpResponse notfound();
    void setContentDisposition(const std::string&filename);
};
