#pragma once

#include "ofMain.h"
#include "ofxAnimatableFloat.h"
#include "ofxAnimatableOfPoint.h"
#include "ofxAnimatableOfColor.h"
#include "ofxAnimatableQueue.h"

class ofxAnimatableQueueOfPoint {

public:
    ofxAnimatableQueueOfPoint() {
        clearQueue();
    }

    struct EventArg {
        ofxAnimatableQueueOfPoint* who;
    };

    void ofxAnimatableQueueOfPoint::setInitialValue(const ofxAnimatableOfPoint val) {
        anim = val;
    }

    void ofxAnimatableQueueOfPoint::addTransition(const ofxAnimatableOfPoint targetValue) {
        animSteps.push_back(targetValue);
    }

    void ofxAnimatableQueueOfPoint::clearQueue() {
        animSteps.clear();
    }
    ofxAnimatableOfPoint ofxAnimatableQueueOfPoint::getCurrentValue() {
        return anim;
    }

    void ofxAnimatableQueueOfPoint::update(float dt) {

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

    ofEvent<ofxAnimatableQueueOfPoint::EventArg> eventQueueDone;

    void startPlaying() {
        currentStep = 0;
        playing = true;
    }

    void pausePlayback() {
        playing = false;
    }

    void resumePlayback() {
        playing = true;
    }

protected:
    size_t currentStep = 0;
    bool playing = false;
private:
    ofxAnimatableOfPoint anim;
    std::vector<ofxAnimatableOfPoint> animSteps;
};


class ofApp : public ofBaseApp{

	public:
        const float fps = 60.0f;
        float r;
        ofLight	light;
        ofEasyCam cam;
        ofxAnimatableFloat blinkerDown;
        ofxAnimatableFloat blinkerUp;
        std::vector<ofImage> eyes;
        ofxAnimatableQueueOfPoint OMD; // Orchestral Manoeuvres in the Dark
        ofxAnimatableQueue eyesHaveIt;
        void onAnimQueueDone(ofxAnimatableQueueOfPoint::EventArg&);
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
    eyes[0].load("p1.jpg");

    eyes.push_back(eye);
    eyes[1].load("blink.png");
    light.setup();
    light.setDiffuseColor(ofColor::white);
    light.setSpecularColor(ofColor::white);
    light.setAmbientColor(ofColor::beige);

    // see size handler too
    windowResized(0,0);

    blinkerDown.setDuration(1.0);
    blinkerDown.setRepeatType(LOOP_BACK_AND_FORTH);
    blinkerDown.setCurve(QUARTIC_EASE_IN);
    blinkerUp.setDuration(1.0);
    blinkerUp.setRepeatType(LOOP_BACK_AND_FORTH);
    blinkerUp.setCurve(QUARTIC_EASE_IN);

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
    //eyesHaveIt.addTransition(pupils);
    //ofAddListener(eyesHaveIt.eventQueueDone, this, &ofApp::onAnimQueueDone);
   // eyesHaveIt.startPlaying();
}

//--------------------------------------------------------------
void ofApp::update(){
    eyes[0].update();
    eyes[1].update();

    OMD.update(1.0f / fps);
    eyesHaveIt.update(1.0f / fps);
    blinkerDown.update(1.0f / fps);
    blinkerUp.update(1.0f / fps);

    // debug helper
    std::stringstream ss;
    ss << blinkerDown.getCurrentValue();
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
    ofPoint p = OMD.getCurrentValue().getCurrentPosition();
    ofScale(1, 1.2, 1); // a bit oblong i figure
    ofRotateXDeg(180); // hide seam
    //ofRotateXDeg(p.x);
   // ofRotateYDeg(p.y);
   // ofRotateZDeg(p.z);
    //ofNoFill();
    ofDrawSphere(0, 0, 0, r);
    unBindEye();
    // blink?
    //ofFill();
    float h = r;
    float w = r * 2;
    ofSetColor(ofColor::black); // need to close up and down
    ofDrawRectangle(-r, -(r + (blinkerDown.getCurrentValue())), -r, w, h);
    ofDrawRectangle(-r, r-(blinkerUp.getCurrentValue()), -r, w, h);

    ofPopStyle();
    ofPopMatrix();
}
// allow for various eyes
void ofApp::unBindEye() {
    eyes[0].getTexture().unbind();
}
void ofApp::bindEye() {
    eyes[0].getTexture().bind();
}
void ofApp::onAnimQueueDone(ofxAnimatableQueueOfPoint::EventArg&) {
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
    light.setAreaLight(ofGetHeight() / 4, ofGetWidth() / 4);
    light.setPosition(-ofGetWidth() / 2, ofGetHeight(), 200);
    float size = std::min(ofGetHeight(), ofGetHeight());
    r = std::min(ofGetHeight(), ofGetHeight()) / 3;
    blinkerUp.animateFromTo(0.0, r);
    blinkerDown.animateFromTo(r, 0.0);
}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}



