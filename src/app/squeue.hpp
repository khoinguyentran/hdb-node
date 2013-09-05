#ifndef SQUEUE_HPP
#define SQUEUE_HPP

#include <boost/property_tree/ptree.hpp>
#include <boost/shared_ptr.hpp>

#include <mutex>
#include <condition_variable>
#include <deque>

using boost::property_tree::ptree;
using boost::shared_ptr;

template < typename T >
class squeue
{
private:
    std::mutex d_mutex;
    std::condition_variable d_condition;
    std::deque<T> d_queue;

public:
    void push ( T const& value )
    {
        {
            std::unique_lock< std::mutex > lock ( this->d_mutex );
            d_queue.push_front ( value );
        }
        this->d_condition.notify_one();
    }

    T pop()
    {
        std::unique_lock< std::mutex > lock ( this->d_mutex );
        this->d_condition.wait ( lock, [=] { return !this->d_queue.empty(); });
        T rc ( std::move ( this->d_queue.back() ) );
        this->d_queue.pop_back();
        return rc;
    }

};

typedef squeue< shared_ptr< ptree > > sequeue;

#endif
