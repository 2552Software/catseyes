#pragma once

#include "ofMain.h"
#include "ofxAnimatableFloat.h"
#include "ofxAnimatableOfPoint.h"
#include "ofxAnimatableOfColor.h"

class ofApp : public ofBaseApp{

	public:
        const float fps = 30.0f;
        ofLight	light;
        ofEasyCam cam;
        ofxAnimatableOfPoint eyeMotion;
        std::vector<int> eyeType;
        std::vector<ofImage> eyes;

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
                ofxAnimatableOfPoint newPhase = animSteps[currentStep];
                //animSteps.erase(animSteps.begin());

                //if (newPhase.getDuration() <= 0.0f) {
                  //  anim = newPhase;
               // }
               // else {
                 //   anim.setCurve(QUADRATIC_EASE_OUT); //get from newPhase if needed
                  //  anim.setDuration(newPhase.getDuration());
                   // anim.animateTo(newPhase.getCurrentPosition());
                //}
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
    light.setup();
    light.setDiffuseColor(ofColor::white);
    light.setSpecularColor(ofColor::white);
    light.setAmbientColor(ofColor::beige);
    light.setAreaLight(ofGetHeight() / 4, ofGetWidth() / 4);
    light.setPosition(-ofGetWidth() / 2, ofGetHeight(), 200);

    eyeMotion.setDuration(1.3);
    eyeMotion.setPosition(ofPoint(0, 0));
    eyeMotion.setRepeatType(LOOP_BACK_AND_FORTH);
    eyeMotion.setCurve(QUADRATIC_EASE_OUT);
    eyeMotion.setRepeatTimes(300);
    eyeMotion.setAutoFlipCurve(true);
    eyeMotion.animateTo(ofPoint(10, 35));

    //we want to know when the animation ends
    ofAddListener(queue.eventQueueDone, this, &ofApp::onAnimQueueDone);
    queue.addTransition(eyeMotion);
    eyeMotion.animateTo(ofPoint(5, 45));
    queue.addTransition(eyeMotion);
    queue.startPlaying(); //start the animation
}

//--------------------------------------------------------------
void ofApp::update(){
    eyes[0].update();

    eyeMotion.update(1.0f / fps);
    queue.update(1.0f / fps);

    // debug helper
    std::stringstream ss;
    ss << queue.getCurrentValue().getCurrentPosition();
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
    ofPoint p = queue.getCurrentValue().getCurrentPosition();
    //ofPoint p = eyeMotion.getCurrentPosition();
    ofRotateXDeg(180); // hide seam
    ofRotateXDeg(p.x);
    ofRotateYDeg(p.y);
    ofRotateZDeg(p.z);
    ofDrawSphere(0, 0, 0, r);
    unBindEye();
    setPupil(-r, 1);
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
    ofSetColor(0, 0, 0, 220);
    float r2;

    switch (type) {
    case 0:
        ofScale(0.5, 2.0, 1.3);
        r2 = r / 4;
        ofDrawSphere(0, 0, r - ((r2*1.9)), r2);
        break;
    case 1:
        ofScale(1, 2.0, 1.3);
        r2 = r / 3.7;
        ofDrawSphere(0, 0, r - ((r2*1.7)), r2);
        break;
    case 2:
        ofScale(1, 1.9, 1.3);
        r2 = r / 3.2;
        ofDrawSphere(0, 0, r - ((r2*1.3)), r2);
        break;
    }

}
void ofApp::onAnimQueueDone(ofxAnimatableQueue2552::EventArg&) {
    queue.startPlaying(); //loop the animation endlessly
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

