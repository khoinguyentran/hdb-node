#ifndef DEVICE_HPP
#define DEVICE_HPP

#include <boost/utility.hpp>
#include <string>

namespace app
{

using std::string;

class device : boost::noncopyable
{
public:
    device ( string const& name ) : name_ ( name ) {}
    ~device() {}
    virtual string desc() const { return "device: " + name_ + " type: " + type_; }
    virtual void start() = 0;
    virtual void stop() = 0;

public:
    virtual string const& name() const { return name_; }
    virtual void name ( string const& value ) { name_ = value; }
    virtual string const& type() const { return type_; }
    virtual void type ( string const& value ) { type_ = value; }
    virtual string const& model() const { return model_; }
    virtual void model ( string const& value ) { model_ = value; }
    virtual string const& location() const { return location_; }
    virtual void location ( string const& value ) { location_ = value; }
    virtual string const& latitude() const { return latitude_; }
    virtual void latitude ( string const& value ) { latitude_ = value; }
    virtual string const& longitude() const { return longitude_; }
    virtual void longitude ( string const& value ) { longitude_ = value; }

protected:
    string name_;
    string type_;
    string model_;
    string location_;
    string latitude_;
    string longitude_;
};

}

#endif
