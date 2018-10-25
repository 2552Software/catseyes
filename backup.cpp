blob = myBlobs.erase(blob);


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
    setLightOrientation(ofVec3f(0.0f, -80.0f, 00.0f));
    material.setShininess(120);
    material.setSpecularColor(ofColor::yellow);
    material.setEmissiveColor(ofColor::cornflowerBlue);
    material.setDiffuseColor(ofColor(255, 255, 255, 255));
    material.setAmbientColor(ofColor::white);

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
    light.setPosition(0, 0, eyeAnimator.camera.getCurrentValue() - 1500);
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







