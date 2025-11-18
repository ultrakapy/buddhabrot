#ifndef CBQUEUE_HH
#define CBQUEUE_HH

#include <deque>
#include <mutex>
#include <condition_variable>

/*!
ConcurrentBoundedQueue is an implementation of a thread-safe queue that uses
a single mutex to guard the queue's contents. Both producers and consumers will 
acquire the mutex before interacting with the queue's internal state.
Details here:
http://courses.cms.caltech.edu/cs11/material/advcpp/bbrot-2/index.html
*/
template<typename T>
class ConcurrentBoundedQueue {
  //! internal data structure used to store queue items
  std::deque<T> q;

  //! upper bound on size of the queue
  int max_items;

  //! guards all gets and puts on the queue
  std::mutex m;

  //! allows consumers to passively wait when the queue is empty
  std::condition_variable cv_q_empty;

  //! allows producers to passively wait when the queue is full
  std::condition_variable cv_q_full;

public:
  //! Constructor specifying the max size of the queue at initialization
  ConcurrentBoundedQueue(int max_items) : max_items(max_items) { };

  //! Adds an item to the front of the queue
  void put(T item);

  //! Removes and returns an item from the back of the queue
  T get();

  /*! Don't allow copy initialization (i.e., copy constructor) since we want
      this queue to be shared and passed around by reference only.
  */
  ConcurrentBoundedQueue(const ConcurrentBoundedQueue&) = delete;

  //! Also don't allow copy assignment due to Rule of Zero
  ConcurrentBoundedQueue& operator=(const ConcurrentBoundedQueue&) = delete;

  //! Also don't allow move constructor due to Rule of Zero
  ConcurrentBoundedQueue(ConcurrentBoundedQueue&&) = delete;

  //! Also don't allow move assignment due to Rule of Zero
  ConcurrentBoundedQueue& operator=(ConcurrentBoundedQueue&&) = delete;

  //! Also don't allow custom destructor due to Rule of Zero
  ~ConcurrentBoundedQueue() = default;
};

template<typename T> inline
void ConcurrentBoundedQueue<T>::put(T item) {
  // acquire mutex lock that guards the queue
  std::unique_lock<std::mutex> lock(m);

  // wait for consumers to consume items if necessary
  while (q.size() == max_items)
    cv_q_full.wait(lock);

  q.push_back(item); // add item to queue
  
  cv_q_empty.notify_all(); // notify all waiting consumers
}

template<typename T> inline
T ConcurrentBoundedQueue<T>::get() {
  // acquire mutex lock that guards the queue
  std::unique_lock<std::mutex> lock(m);

  // wait for producers to produce items if necessary
  while (q.empty())
    cv_q_empty.wait(lock);

  // get then pop front item from queue
  T front_item = q.front();
  q.pop_front();
  
  cv_q_full.notify_all(); // notify all waiting producers

  return front_item;
}


#endif // CBQUEUE_HH


