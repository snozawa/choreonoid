/**
   @file
   @author Shin'ichiro Nakaoka
*/

#include "SceneCollision.h"
#include <cnoid/SceneVisitor>

using namespace std;
using namespace cnoid;

SceneCollision::SceneCollision(boost::shared_ptr< std::vector<CollisionLinkPairPtr> > collisionPairs)
    : collisionPairs(collisionPairs)
{
    vertices_ = setVertices(new SgVertexArray);
    setMaterial(new SgMaterial)->setDiffuseColor(Vector3f(0.0f, 1.0f, 0.0f));
    isDirty = true;
}


SceneCollision::SceneCollision(const SceneCollision& org)
{

}


void SceneCollision::accept(SceneVisitor& visitor)
{
    if(!visitor.property()->get("collision", false)){
        visitor.visitNode(this);
        return;
    }
    
    if(isDirty){
        vertices_->clear();
        lineVertices().clear();
        for(size_t i=0; i < collisionPairs->size(); ++i){
            const vector<Collision>& cols = (*collisionPairs)[i]->collisions;
            for(size_t j=0; j < cols.size(); ++j){
                const Collision& c = cols[j];
                const int index = vertices_->size();
                addLine(index, index + 1);
                vertices_->push_back(c.point.cast<float>());
                vertices_->push_back((c.point + 50.0 * c.depth * c.normal).cast<float>());
            }
        }
        isDirty = false;
    }
    
    visitor.visitLineSet(this);
}
