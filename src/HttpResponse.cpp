#include <string>
#include "HttpResponse.h"
#include <fstream>
#include <sstream>

    HttpResponse::HttpResponse(int status=200,const std::string&content_type="text/plain")
        :status_code(status),content_type(content_type){}
        void HttpResponse::SetBody(const std::string &body){body_=body;}

    std::string HttpResponse::build()const{
        std::ostringstream response;
        std::string status_text=(status_code==200)?"OK":"Not found";
        response<<"HTTP/1.1 " << status_code << " " << status_text << "\r\n";
        response<<"Content-Type: "<<content_type<<"\r\n";
        response<<"Content-Length: "<<body_.size()<<"\r\n";
        if (!content_disposition.empty()){
            response<<"Content-Disposition: "<<content_disposition<<"\r\n";
        }
        response<<"Connection : close\r\n\r\n";
        response<<body_;
        return response.str();
    }
     HttpResponse HttpResponse::notfound(){
        HttpResponse resp(400,"text/html");
        resp.SetBody("<html><body><h1> Peace love Empathy ,bye,bye\nNot found</h1><body></html>");
        return resp;
    }
     HttpResponse HttpResponse::json(const std::string &data){
        HttpResponse resp(200,"application/json");
        resp.SetBody(data);
        return resp;
    }
    void HttpResponse::setContentDisposition(const std::string &filename){
        content_disposition="attachment; filename=\""+filename+"\"";
    }
