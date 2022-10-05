#ifndef SINGLETON_H
#define SINGLETON_H

//--------------------------------------------------------------------------------------------------------------
// Singleton
//--------------------------------------------------------------------------------------------------------------

template <class T>
class Singleton {
public:
  Singleton(Singleton const&) = delete;
  void operator = (Singleton const&) = delete;

  static T* getInstance() {
    if (!pInstance_)
      pInstance_ = new T();

    return pInstance_;
  }

protected:
  Singleton() {}
  ~Singleton() {
    if (pInstance_) {
      delete pInstance_;
      pInstance_ = nullptr;
    }
  }

private:
  static T* pInstance_;
};

template <class T>
T* Singleton<T>::pInstance_ = NULL;

#endif // SINGLETON_H
