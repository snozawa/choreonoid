/**
   @file
   @author Shin'ichiro Nakaoka
*/

#ifndef CNOID_UTIL_MULTI_VECTOR3_SEQ_H_INCLUDED
#define CNOID_UTIL_MULTI_VECTOR3_SEQ_H_INCLUDED

#include "MultiSeq.h"
#include "EigenTypes.h"
#include "exportdecl.h"

namespace cnoid {

class Mapping;
class YAMLWriter;

class CNOID_EXPORT MultiVector3Seq : public MultiSeq<Vector3, Eigen::aligned_allocator<Vector3> >
{
    typedef MultiSeq<Vector3, Eigen::aligned_allocator<Vector3> > BaseSeqType;

public:
    typedef boost::shared_ptr<MultiVector3Seq> Ptr;

    MultiVector3Seq();
    MultiVector3Seq(int numFrames, int numParts = 1);
    MultiVector3Seq(const MultiVector3Seq& org);
    virtual ~MultiVector3Seq();

    virtual AbstractSeqPtr cloneSeq() const;
    void copySeqProperties(const MultiVector3Seq& source);
        
protected:
    virtual Vector3 defaultValue() const { return Vector3::Zero(); }

    virtual bool doWriteSeq(YAMLWriter& writer);
    virtual bool doReadSeq(const Mapping& archive);
};

typedef MultiVector3Seq::Ptr MultiVector3SeqPtr;        
}

#endif
