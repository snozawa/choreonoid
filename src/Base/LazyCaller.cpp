/**
   @author Shin'ichiro Nakaoka
*/

#include "LazyCaller.h"
#include <QObject>
#include <QEvent>
#include <QCoreApplication>
#include <QThread>
#include <QSemaphore>
#include <boost/make_shared.hpp>
#include <boost/bind.hpp>

using namespace std;
using namespace cnoid;

namespace {

inline int toQtPriority(int priority) {
    if(priority <= LazyCaller::PRIORITY_HIGH){
        return Qt::HighEventPriority;
    } else if(priority >= LazyCaller::PRIORITY_LOW){
        return Qt::LowEventPriority;
    } else {
        return Qt::NormalEventPriority;
    }
}

class SyncInfo
{
public:
    QSemaphore semaphore;
    bool completed;
    SyncInfo() {
        completed = false;
    }
};
typedef boost::shared_ptr<SyncInfo> SyncInfoPtr;
    

class CallEvent : public QEvent
{
public:
    CallEvent(const boost::function<void(void)>& function)
        : QEvent(QEvent::User),
          function(function) {
    }
    CallEvent(const boost::function<void(void)>& function, SyncInfoPtr& syncInfo)
        : QEvent(QEvent::User),
          function(function),
          syncInfo(syncInfo) {
    }
    CallEvent(const CallEvent& org)
        : QEvent(QEvent::User),
          function(org.function),
          syncInfo(org.syncInfo) {
    }
    ~CallEvent() {
        if(syncInfo){
            syncInfo->semaphore.release(); // wake up the caller process
        }
    }
    boost::function<void(void)> function;
    SyncInfoPtr syncInfo;
};
    

class CallEventHandler : public QObject
{
public:
    CallEventHandler() {
        mainThreadId = QThread::currentThreadId();
    }
    virtual bool event(QEvent* e);
    Qt::HANDLE mainThreadId;
};
    
CallEventHandler callEventHandler;
}

namespace cnoid {

class LazyCallerImpl : public QObject
{
public:
    LazyCaller* self;
    boost::function<void(void)> function;
    int priority;
    bool isConservative;
    LazyCallerImpl(LazyCaller* self);
    LazyCallerImpl(LazyCaller* self, const boost::function<void(void)>& function, int priority);
    virtual bool event(QEvent* e);
};
}


bool cnoid::isRunningInMainThread()
{
    return (QThread::currentThreadId() == callEventHandler.mainThreadId);
}


void cnoid::callLater(const boost::function<void(void)>& function, int priority)
{
    CallEvent* event = new CallEvent(function);
    QCoreApplication::postEvent(&callEventHandler, event, toQtPriority(priority));
}


bool cnoid::callSynchronously(const boost::function<void(void)>& function, int priority)
{
    if(QThread::currentThreadId() == callEventHandler.mainThreadId){
        function();
        //callLater(function, priority);
        return true;
    } else {
        SyncInfoPtr syncInfo = boost::make_shared<SyncInfo>();
        QCoreApplication::postEvent(
            &callEventHandler, new CallEvent(function, syncInfo), toQtPriority(priority));
        syncInfo->semaphore.acquire(); // wait for finish
        return syncInfo->completed;
    }
}


bool CallEventHandler::event(QEvent* e)
{
    CallEvent* callEvent = dynamic_cast<CallEvent*>(e);
    if(callEvent){
        callEvent->function();
        if(callEvent->syncInfo){
            callEvent->syncInfo->completed = true;
        }
        return true;
    }
    return false;
}


LazyCaller::LazyCaller()
{
    isPending_ = false;
    impl = new LazyCallerImpl(this);
}


LazyCallerImpl::LazyCallerImpl(LazyCaller* self)
    : self(self)
{
    priority = LazyCaller::PRIORITY_HIGH;
    isConservative = false;
}
    

LazyCaller::LazyCaller(const boost::function<void(void)>& function, int priority)
{
    isPending_ = false;
    impl = new LazyCallerImpl(this, function, priority);
}


LazyCallerImpl::LazyCallerImpl(LazyCaller* self, const boost::function<void(void)>& function, int priority)
    : self(self),
      function(function),
      priority(priority)
{
    isConservative = false;
}


LazyCaller::LazyCaller(const LazyCaller& org)
{
    isPending_ = false;
    impl = new LazyCallerImpl(this, org.impl->function, org.impl->priority);
    impl->isConservative = org.impl->isConservative;
}


LazyCaller::~LazyCaller()
{
    if(isPending_){
        QCoreApplication::removePostedEvents(impl);
        isPending_ = false;
    }
    delete impl;
}


void LazyCaller::setFunction(const boost::function<void(void)>& function)
{
    impl->function = function;
}


void LazyCaller::setPriority(int priority)
{
    impl->priority = priority;
}


void LazyCaller::setConservative(bool on)
{
    impl->isConservative = on;
}


void LazyCaller::postCallEvent()
{
    CallEvent* event = new CallEvent(impl->function);
    QCoreApplication::postEvent(impl, event, toQtPriority(impl->priority));
}


bool LazyCallerImpl::event(QEvent* e)
{
    CallEvent* callEvent = dynamic_cast<CallEvent*>(e);
    if(callEvent){
        if(isConservative){
            callEvent->function();
            self->isPending_ = false;
        } else {
            self->isPending_ = false;
            callEvent->function();
        }
        return true;
    }
    return false;
}
