/*  Sirikata
 *  SimpleCameraObjectScript.cpp
 *
 *  Copyright (c) 2010, Ewen Cheslack-Postava
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are
 *  met:
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *  * Neither the name of Sirikata nor the names of its contributors may
 *    be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
 * IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER
 * OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */


#include <sirikata/oh/Platform.hpp>

#include <sirikata/core/util/KnownServices.hpp>


#include "BrainObjectScript.hpp"
using namespace Sirikata;
namespace Elysia {

#define WORLD_SCALE 20  // Units for zoom/pan
#define DEG2RAD 0.0174532925

BrainObjectScript::BrainObjectScript(HostedObjectPtr ho, const String& args)
 : mParent(ho)
{
    mParent->addListener((SessionEventListener*)this);
    std::cout<<"INITIALIZE A SINGLE BRAIN SCRIPT\n";
/*
    // Setup input responses
    mInputResponses["suspend"] = new SimpleInputResponse(std::tr1::bind(&BrainObjectScript::suspendAction, this));
    mInputResponses["resume"] = new SimpleInputResponse(std::tr1::bind(&BrainObjectScript::resumeAction, this));
    mInputResponses["toggleSuspend"] = new SimpleInputResponse(std::tr1::bind(&BrainObjectScript::toggleSuspendAction, this));
    mInputResponses["quit"] = new SimpleInputResponse(std::tr1::bind(&BrainObjectScript::quitAction, this));
    mInputResponses["screenshot"] = new SimpleInputResponse(std::tr1::bind(&BrainObjectScript::screenshotAction, this));

    mInputResponses["moveForward"] = new FloatToggleInputResponse(std::tr1::bind(&BrainObjectScript::moveAction, this, Vector3f(0, 0, -1), _1), 1, 0);
    mInputResponses["moveBackward"] = new FloatToggleInputResponse(std::tr1::bind(&BrainObjectScript::moveAction, this, Vector3f(0, 0, 1), _1), 1, 0);
    mInputResponses["moveLeft"] = new FloatToggleInputResponse(std::tr1::bind(&BrainObjectScript::moveAction, this, Vector3f(-1, 0, 0), _1), 1, 0);
    mInputResponses["moveRight"] = new FloatToggleInputResponse(std::tr1::bind(&BrainObjectScript::moveAction, this, Vector3f(1, 0, 0), _1), 1, 0);
    mInputResponses["moveDown"] = new FloatToggleInputResponse(std::tr1::bind(&BrainObjectScript::moveAction, this, Vector3f(0, -1, 0), _1), 1, 0);
    mInputResponses["moveUp"] = new FloatToggleInputResponse(std::tr1::bind(&BrainObjectScript::moveAction, this, Vector3f(0, 1, 0), _1), 1, 0);

    mInputResponses["rotateXPos"] = new FloatToggleInputResponse(std::tr1::bind(&BrainObjectScript::rotateAction, this, Vector3f(1, 0, 0), _1), 1, 0);
    mInputResponses["rotateXNeg"] = new FloatToggleInputResponse(std::tr1::bind(&BrainObjectScript::rotateAction, this, Vector3f(-1, 0, 0), _1), 1, 0);
    mInputResponses["rotateYPos"] = new FloatToggleInputResponse(std::tr1::bind(&BrainObjectScript::rotateAction, this, Vector3f(0, 1, 0), _1), 1, 0);
    mInputResponses["rotateYNeg"] = new FloatToggleInputResponse(std::tr1::bind(&BrainObjectScript::rotateAction, this, Vector3f(0, -1, 0), _1), 1, 0);
    mInputResponses["rotateZPos"] = new FloatToggleInputResponse(std::tr1::bind(&BrainObjectScript::rotateAction, this, Vector3f(0, 0, 1), _1), 1, 0);
    mInputResponses["rotateZNeg"] = new FloatToggleInputResponse(std::tr1::bind(&BrainObjectScript::rotateAction, this, Vector3f(0, 0, -1), _1), 1, 0);

    mInputResponses["stableRotatePos"] = new FloatToggleInputResponse(std::tr1::bind(&BrainObjectScript::stableRotateAction, this, 1.f, _1), 1, 0);
    mInputResponses["stableRotateNeg"] = new FloatToggleInputResponse(std::tr1::bind(&BrainObjectScript::stableRotateAction, this, -1.f, _1), 1, 0);


    mInputBinding.addFromFile("keybinding.default", mInputResponses);
*/
}

BrainObjectScript::~BrainObjectScript()
{
    mParent->removeListener((SessionEventListener*)this);
/*
    for (InputBinding::InputResponseMap::iterator iter = mInputResponses.begin(), iterend = mInputResponses.end(); iter != iterend; ++iter)
        delete iter->second;
*/
}

Context* BrainObjectScript::context() const {
    return mParent->context();
}

void BrainObjectScript::updateAddressable()
{
}

void BrainObjectScript::attachScript(const String& script_name)
{
}

void BrainObjectScript::onConnected(SessionEventProviderPtr from, const SpaceObjectReference& name, int64 token) {
    mID = name;
    mSelfProxy = mParent->self(mID);
    moveAction(Vector3f(0,0,1),10);
    
}

void BrainObjectScript::onDisconnected(SessionEventProviderPtr from, const SpaceObjectReference& name) {
}


static String fillZeroPrefix(const String& prefill, int32 nwide) {
    String retval = prefill;
    while((int)retval.size() < nwide)
        retval = String("0") + retval;
    return retval;
}

void BrainObjectScript::moveAction(Vector3f dir, float amount)
{
    // Get the updated position
    Time now = context()->simTime();
    Location loc = mSelfProxy->extrapolateLocation(now);
    const Quaternion &orient = loc.getOrientation();

    // Request updates from spcae
    TimedMotionVector3f newloc(now, MotionVector3f(Vector3f(loc.getPosition()), (orient * dir) * amount * WORLD_SCALE * .5) );
    mParent->requestLocationUpdate(mID.space(), mID.object(), newloc);
    // And update our local Proxy's information, assuming the move will be successful
    mSelfProxy->setLocation(newloc, 90000, true);
}

void BrainObjectScript::rotateAction(Vector3f about, float amount)
{
    // Get the updated position
    Time now = context()->simTime();
    Location loc = mSelfProxy->extrapolateLocation(now);
    const Quaternion &orient = loc.getOrientation();

    TimedMotionQuaternion neworient(now, MotionQuaternion(loc.getOrientation(), Quaternion(about, amount)));
    mParent->requestOrientationUpdate(mID.space(), mID.object(), neworient);
    // And update our local Proxy's information, assuming the move will be successful
    mSelfProxy->setOrientation(neworient, 0, true);
}

static bool quat2Euler(Quaternion q, double& pitch, double& roll, double& yaw) {
    /// note that in the 'gymbal lock' situation, we will get nan's for pitch.
    /// for now, in that case we should revert to quaternion
    double q1,q2,q3,q0;
    q2=q.x;
    q3=q.y;
    q1=q.z;
    q0=q.w;
    roll = std::atan2((2*((q0*q1)+(q2*q3))), (1-(2*(std::pow(q1,2.0)+std::pow(q2,2.0)))));
    pitch = std::asin((2*((q0*q2)-(q3*q1))));
    yaw = std::atan2((2*((q0*q3)+(q1*q2))), (1-(2*(std::pow(q2,2.0)+std::pow(q3,2.0)))));
    pitch /= DEG2RAD;
    roll /= DEG2RAD;
    yaw /= DEG2RAD;
    if (std::abs(pitch) > 89.0) {
        return false;
    }
    return true;
}


void BrainObjectScript::stableRotateAction(float dir, float amount)
{
    // Get the updated position
    Time now = context()->simTime();
    Location loc = mSelfProxy->extrapolateLocation(now);
    const Quaternion &orient = loc.getOrientation();

    double p, r, y;
    quat2Euler(orient, p, r, y);
    Vector3f raxis;
    raxis.x = 0;
    raxis.y = std::cos(p*DEG2RAD);
    raxis.z = -std::sin(p*DEG2RAD);

    // Request updates from spcae
    TimedMotionQuaternion neworient(now, MotionQuaternion(loc.getOrientation(), Quaternion(raxis, dir*amount)));
    mParent->requestOrientationUpdate(mID.space(), mID.object(), neworient);
    // And update our local Proxy's information, assuming the move will be successful
    mSelfProxy->setOrientation(neworient, 0, true);
}

} // namespace Elysia