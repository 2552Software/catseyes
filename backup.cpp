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
                    if (infinite) {
                        currentStep = 0;
                        anim = animSteps[currentStep];
                    }
                    else {
                        playing = false;
                    }
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
    bool infinite = true;
};

class ImageAnimator {
public:
    void setup() {
        animator.reset(0.0f);
        animator.setDuration(2.0f);
        animator.setRepeatType(LOOP);
        animator.setCurve(LINEAR);

        camera.reset(500.0f);
        camera.setDuration(5.0f);
        camera.setRepeatType(LOOP_BACK_AND_FORTH);
        camera.setCurve(EASE_IN);

        color.setColor(ofColor::white);
        color.setDuration(1.0f);
        color.setRepeatType(LOOP_BACK_AND_FORTH);
        color.setCurve(LINEAR);
        color.animateTo(ofColor::orangeRed);
    }
    void startPlaying() {
       setup();
       string path = "";
       ofDirectory dir(path);
       dir.allowExt("png");
       dir.allowExt("jpg");
       dir.listDir();
       for (size_t i = 0; i < dir.size(); i++) {
           add(dir.getPath(i));
       }

       animator.animateTo(images.size()); 
    }
    void add(const std::string &name) {
        ofImage image;
        if (image.load(name)){
            images.push_back(image);
        }
    }
    void add(ofImage image) {
        images.push_back(image);
    }
    ofImage& getImage() {
        return images[getIndex()];
    }
    int getIndex() {
        return (int)animator.getCurrentValue();
    }
    void update(float f) {
        animator.update(f);
        color.update(f);
        camera.update(f);
        for (ofImage& image : images) {
            image.update(); // keep updated
        }
    }
    // allow for various eyes
    void unbind() {
        getImage().getTexture().unbind();
    }
    void bind() {
        color.applyCurrentColor();
        ofRotateZDeg(180.0);
        getImage().getTexture().bind();
    }
    ofxAnimatableFloat camera;
    ofxAnimatableFloat animator; 
    ofxAnimatableOfColor color; // image background color
private:
    std::vector<ofImage> images;
};

class MotionGenerator {
public:
    void draw() {
        ofPoint p = queue.getCurrentValue().getCurrentPosition();
        ofRotateXDeg(p.x);
        ofRotateYDeg(p.y);
        ofRotateZDeg(p.z);
    }
    void add(const ofPoint& start, const ofPoint& target) {
        ofxAnimatableOfPoint targetPoint;
        targetPoint.setPosition(start);
        targetPoint.animateTo(target);
        targetPoint.setDuration(2.3);
        targetPoint.setRepeatType(PLAY_ONCE);
        targetPoint.setCurve(QUADRATIC_EASE_OUT);
        queue.addTransition(targetPoint);
    }
    void update(float f) {
        queue.update(f);
    }
    void startPlaying() {
        queue.startPlaying();
    }

    ofxAnimatableQueueOfPoint queue;
};

class ofApp : public ofBaseApp{

	public:
        const float fps = 60.0f;
        float r;
        ofLight	light;
        ofEasyCam camera;
        MotionGenerator motion;
        ImageAnimator eyeAnimator;
        ofMaterial material;
		void setup();
		void update();
		void draw();
        void onAnimQueueDone(ofxAnimatableQueueOfPoint::EventArg&) {
            motion.startPlaying(); //loop the animation endlessly
        }
        void setLightOrientation(ofVec3f rot)       {
            ofVec3f xax(1, 0, 0);
            ofVec3f yax(0, 1, 0);
            ofVec3f zax(0, 0, 1);
            ofQuaternion q;
            q.makeRotate(rot.x, xax, rot.y, yax, rot.z, zax);
            light.setOrientation(q);
        }
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

    eyeAnimator.startPlaying();

    light.setup();
    light.setDiffuseColor(ofFloatColor(255.0, 0.0, 0.0f));
    light.setSpecularColor(ofColor(0, 0, 255));
    light.setDirectional();
    setLightOrientation(ofVec3f(0.0f, -50.0f, -500.0f));
    material.setShininess(120);
    material.setSpecularColor(ofColor(255, 255, 255, 255));
    material.setEmissiveColor(ofColor(0, 0, 0, 255));
    material.setDiffuseColor(ofColor(255, 255, 255, 255));
    material.setAmbientColor(ofColor(255, 255, 255, 255));

    // see size handler too
    windowResized(0,0);

    motion.add(ofPoint(0.0f, 0.0f), ofPoint(10.0f, 90.0f));
    motion.add(ofPoint(10.0f, 90.0f), ofPoint(-10.0f, 15.0f));
    motion.add(ofPoint(-10.0f, 15.0f), ofPoint(0.0f, 0.0f));
    motion.startPlaying(); //start the animation
}

//--------------------------------------------------------------
void ofApp::update(){
    motion.update(1.0f / fps);
    eyeAnimator.update(1.0f / fps);
    camera.setDistance(eyeAnimator.camera.getCurrentValue());

    // debug helper
    std::stringstream ss;
    ss << eyeAnimator.getIndex();
    ofSetWindowTitle(ss.str());
}

//--------------------------------------------------------------
void ofApp::draw(){
    ofTranslate(ofGetWidth() / 2, ofGetHeight() / 2, 0);
    ofPushMatrix();
    ofPushStyle();
    light.enable();
    material.begin();
    camera.begin();
    eyeAnimator.bind();
    ofScale(1.0f, 1.0f, 1.0f); // a bit oblong i figure
    ofRotateXDeg(180.0f); // hide seam
    motion.draw();
    ofDrawSphere(0.0f, 0.0f, 0.0f, r);
    eyeAnimator.unbind();
    camera.end();
    material.end();
    light.disable();
    ofPopStyle();
    ofPopMatrix();
}
//--------------------------------------------------------------
void ofApp::windowResized(int w, int h) {

    float size = std::min(ofGetHeight(), ofGetHeight());
    r = std::min(ofGetHeight(), ofGetHeight()) / 3;
    eyeAnimator.camera.reset(ofGetHeight());
    eyeAnimator.camera.animateTo(ofGetHeight() / 2);
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
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}






