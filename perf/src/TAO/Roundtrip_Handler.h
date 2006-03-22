//
// Roundtrip_Handler.h,v 1.4 2003/07/21 23:51:34 dhinton Exp
//

#ifndef ROUNDTRIP_HANDLER_H
#define ROUNDTRIP_HANDLER_H
#include /**/ "ace/pre.h"

#include "TestS.h"
#include "ace/Basic_Stats.h"

#if defined (_MSC_VER)
# if (_MSC_VER >= 1200)
#  pragma warning(push)
# endif /* _MSC_VER >= 1200 */
# pragma warning (disable:4250)
#endif /* _MSC_VER */

#include <IceUtil/Mutex.h>
#include <IceUtil/Monitor.h>

/// Implement the Test::Roundtrip interface
class Roundtrip_Handler
  : public virtual POA_Test::AMI_RoundtripHandler
  , public virtual PortableServer::RefCountServantBase
  , public IceUtil::Monitor<IceUtil::Mutex>
{
public:
  /// Constructor
  Roundtrip_Handler();

  void waitFinished();

  // = The skeleton methods
  virtual void sendByteSeq (ACE_ENV_SINGLE_ARG_DECL)
    ACE_THROW_SPEC ((CORBA::SystemException));
  virtual void sendByteSeq_excep(Messaging::ExceptionHolder *holder
  				 ACE_ENV_ARG_DECL)
    ACE_THROW_SPEC ((CORBA::SystemException));

  virtual void sendStringSeq (ACE_ENV_SINGLE_ARG_DECL)
    ACE_THROW_SPEC ((CORBA::SystemException));
  virtual void sendStringSeq_excep(Messaging::ExceptionHolder *holder
  				 ACE_ENV_ARG_DECL)
    ACE_THROW_SPEC ((CORBA::SystemException));

  virtual void sendStringDoubleSeq (ACE_ENV_SINGLE_ARG_DECL)
    ACE_THROW_SPEC ((CORBA::SystemException));
  virtual void sendStringDoubleSeq_excep(Messaging::ExceptionHolder *holder
  				 ACE_ENV_ARG_DECL)
    ACE_THROW_SPEC ((CORBA::SystemException));

  virtual void test_method (ACE_ENV_SINGLE_ARG_DECL)
    ACE_THROW_SPEC ((CORBA::SystemException));
  virtual void test_method_excep (Messaging::ExceptionHolder *holder
                                  ACE_ENV_ARG_DECL)
    ACE_THROW_SPEC ((CORBA::SystemException));

  virtual void shutdown (ACE_ENV_SINGLE_ARG_DECL)
    ACE_THROW_SPEC ((CORBA::SystemException));

  virtual void shutdown_excep (Messaging::ExceptionHolder *holder
                                  ACE_ENV_ARG_DECL)
    ACE_THROW_SPEC ((CORBA::SystemException));

private:

  bool _finished;
  
};

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma warning(pop)
#endif /* _MSC_VER */

#include /**/ "ace/post.h"
#endif /* ROUNDTRIP_H */
