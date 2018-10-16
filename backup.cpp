#pragma once

#include "ofMain.h"
#include "ofxAnimatableFloat.h"
#include "ofxAnimatableOfPoint.h"
#include "ofxAnimatableOfColor.h"

class ofxAnimatableQueue2552 {

public:
    ofxAnimatableQueue2552() {
        clearQueue();
    }

    struct EventArg {
        ofxAnimatableQueue2552* who;
    };

    void setInitialValue(const ofxAnimatableOfPoint val); //initial value of the timeline
    void addTransition(const ofxAnimatableOfPoint targetValue);
    void clearQueue(); //removes all transitions

    ofxAnimatableOfPoint getCurrentValue();

    void startPlaying(); //resets playhead to 0
    void pausePlayback(); //just stops updating
    void resumePlayback(); //just resumes updating
    bool isPlaying() { return playing; }


    void update(float dt);

    ofEvent<ofxAnimatableQueue2552::EventArg> eventQueueDone;

protected:

    std::vector<ofxAnimatableOfPoint> animSteps;
    int currentStep = 0;
    bool playing = false;

    ofxAnimatableOfPoint anim;

};

class ofApp : public ofBaseApp{

	public:
        const float fps = 30.0f;
        ofLight	light;
        ofEasyCam cam;
        ofxAnimatableOfPoint eyeMotion; // add some color and eye size at some point, also a crazy eyes
        std::vector<int> eyeType;
        std::vector<ofImage> eyes;
        ofxAnimatableQueue2552 queue;
        void onAnimQueueDone(ofxAnimatableQueue2552::EventArg&);
		void setup();
		void update();
		void draw();

		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void mouseEntered(int x, int y);
		void mouseExited(int x, int y);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);

private:
    void setPupil(float r, int type = 0);
    void bindEye();
    void unBindEye();
};

#pragma once

#include "ofMain.h"
#include "ofxAnimatableFloat.h"
#include "ofxAnimatableOfPoint.h"
#include "ofxAnimatableOfColor.h"

class ofxAnimatableQueue2552 {

public:
    ofxAnimatableQueue2552() {
        clearQueue();
    }

    struct EventArg {
        ofxAnimatableQueue2552* who;
    };

    void setInitialValue(const ofxAnimatableOfPoint val); //initial value of the timeline
    void addTransition(const ofxAnimatableOfPoint targetValue);
    void clearQueue(); //removes all transitions

    ofxAnimatableOfPoint getCurrentValue();

    void startPlaying(); //resets playhead to 0
    void pausePlayback(); //just stops updating
    void resumePlayback(); //just resumes updating
    bool isPlaying() { return playing; }

    void update(float dt);

    ofEvent<ofxAnimatableQueue2552::EventArg> eventQueueDone;

protected:

private:
    ofxAnimatableOfPoint anim;
    std::vector<ofxAnimatableOfPoint> animSteps;
    size_t currentStep = 0;
    bool playing = false;
};


class ofApp : public ofBaseApp{

	public:
        const float fps = 60.0f;
        ofLight	light;
        ofEasyCam cam;
        std::vector<int> eyeType;
        std::vector<ofImage> eyes;
        ofxAnimatableQueue2552 OMD; // Orchestral Manoeuvres in the Dark
        ofxAnimatableQueue2552 eyesHaveIt;
        void onAnimQueueDone(ofxAnimatableQueue2552::EventArg&);
		void setup();
		void update();
		void draw();

		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void mouseEntered(int x, int y);
		void mouseExited(int x, int y);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);

private:
    void setPupil(float r, int type = 0);
    void bindEye();
    void unBindEye();
};
#include "ofApp.h"

void ofxAnimatableQueue2552::setInitialValue(const ofxAnimatableOfPoint val) {
    anim = val;
}

void ofxAnimatableQueue2552::addTransition(const ofxAnimatableOfPoint targetValue) {
    animSteps.push_back(targetValue);
}

void ofxAnimatableQueue2552::clearQueue() {
    animSteps.clear();
}

void ofxAnimatableQueue2552::update(float dt) {

    if (playing) {

        anim.update(dt);

        if (anim.hasFinishedAnimating()) {
            if (currentStep < animSteps.size()) {
                //animSteps.erase(animSteps.begin());
                anim = animSteps[currentStep];
                currentStep++;
            }
            else {
                playing = false;
                EventArg arg = { this };
                ofNotifyEvent(eventQueueDone, arg);
            }
        }
    }
}


ofxAnimatableOfPoint ofxAnimatableQueue2552::getCurrentValue() {
    return anim;
}

void ofxAnimatableQueue2552::startPlaying() {
    currentStep = 0;
    playing = true;
}

void ofxAnimatableQueue2552::pausePlayback() {
    playing = false;
}

void ofxAnimatableQueue2552::resumePlayback() {
    playing = true;
}
//
//--------------------------------------------------------------
void ofApp::setup(){
    ofSetFrameRate(fps);
    ofBackground(ofColor::black);
    ofEnableLighting();
    ofEnableDepthTest();
    ofSetVerticalSync(true);
    ofDisableArbTex();
    ofSetSmoothLighting(true);
    //ofDisableAlphaBlending();
    cam.setDistance(700);
    light.enable();
    ofImage eye;
    eyes.push_back(eye);
    eyes[0].load("orange3a.jpg");

    eyes.push_back(eye);
    eyes[1].load("p1.jpg");

    light.setup();
    light.setDiffuseColor(ofColor::white);
    light.setSpecularColor(ofColor::white);
    light.setAmbientColor(ofColor::beige);
    light.setAreaLight(ofGetHeight() / 4, ofGetWidth() / 4);
    light.setPosition(-ofGetWidth() / 2, ofGetHeight(), 200);

    ofxAnimatableOfPoint eyeMotion; // add some color and eye size at some point, also a crazy eyes
    eyeMotion.setDuration(0.3);
    eyeMotion.setPosition(ofPoint(0, 0));
    eyeMotion.setRepeatType(PLAY_N_TIMES);
    eyeMotion.setCurve(QUADRATIC_EASE_OUT);
    eyeMotion.setRepeatTimes(20);
    eyeMotion.animateTo(ofPoint(0, 0, 360.5));

    //we want to know when the animation ends
    ofAddListener(OMD.eventQueueDone, this, &ofApp::onAnimQueueDone);

    OMD.addTransition(eyeMotion);
    
    eyeMotion.setPosition(ofPoint(10, 35));
    eyeMotion.animateTo(ofPoint(15, 90));
    OMD.addTransition(eyeMotion);

    OMD.startPlaying(); //start the animation

    ofxAnimatableOfPoint pupils;
    pupils.setDuration(0.3);
    pupils.setPosition(ofPoint(0, 0, 2));
    pupils.setRepeatType(PLAY_N_TIMES);
    pupils.setCurve(QUADRATIC_EASE_OUT);
    pupils.setRepeatTimes(1);
    pupils.animateTo(ofPoint(0, 0, 5));
    eyesHaveIt.addTransition(pupils);
    ofAddListener(eyesHaveIt.eventQueueDone, this, &ofApp::onAnimQueueDone);
    eyesHaveIt.startPlaying();
}

//--------------------------------------------------------------
void ofApp::update(){
    eyes[0].update();
    eyes[1].update();

    OMD.update(1.0f / fps);
    eyesHaveIt.update(1.0f / fps);

    // debug helper
    std::stringstream ss;
    ss << OMD.getCurrentValue().getCurrentPosition();
    ofSetWindowTitle(ss.str());

}

//--------------------------------------------------------------
void ofApp::draw(){
    ofTranslate(ofGetWidth() / 2, ofGetHeight() / 2, 0);
    ofPushMatrix();
    ofPushStyle();
    cam.begin();
    bindEye();
    ofSetColor(ofColor::white);
    float size = std::min(ofGetHeight(), ofGetHeight());
    float r = size / 3;
    ofPoint p = OMD.getCurrentValue().getCurrentPosition();
    ofScale(1, 1.3, 1.3); // a bit oblong i figure
    ofRotateXDeg(180); // hide seam
    //ofRotateXDeg(p.x);
   // ofRotateYDeg(p.y);
   // ofRotateZDeg(p.z);
    ofDrawSphere(0, 0, 0, r);
    unBindEye();
    setPupil(r, 2);
    ofPopStyle();
    ofPopMatrix();
}
// allow for various eyes
void ofApp::unBindEye() {
    eyes[0].getTexture().unbind();
}
void ofApp::bindEye() {
    eyes[0].getTexture().bind(); // allow for various eyes
}
void ofApp::setPupil(float r, int type) {
    // have different types, sizes, colors etc
    ofSetColor(0, 254, 0, 220);
    float r2, z=0.0;
    //ofScale(1, 2.0, 1.3); // x,y go here
    ofPoint p = eyesHaveIt.getCurrentValue().getCurrentPosition();
    r2 = r / p.z;
    ofDrawCircle(0, 0, r2);
    switch (type) {
    case 0:
       // r2 = r / 4;
        break;
    case 1:
        //r2 = r / 3;
        break;
    case 2:
        //r2 = r / 2.0;
        break;
    }
    //ofDrawSphere(0, 0, r + ((r2*1.7)), abs(r2));
    //ofDrawSphere(0, 0, r, r2);

}
void ofApp::onAnimQueueDone(ofxAnimatableQueue2552::EventArg&) {
    OMD.startPlaying(); //loop the animation endlessly
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}


