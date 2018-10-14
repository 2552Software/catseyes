#include "ofApp.h"


#pragma once

#include "ofMain.h"

class ofApp : public ofBaseApp{

	public:
        ofLight	light;
        ofEasyCam cam;

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

        std::vector<glm::vec3> Movements;
        std::vector<int> eyeType;
        std::vector<ofImage> eyes;

private:
    void setPupil(float r, int type = 0);
    void bindEye();
    void unBindEye();
		
};

//--------------------------------------------------------------
#include "ofApp.h"
//
//--------------------------------------------------------------
void ofApp::setup(){
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
}

//--------------------------------------------------------------
void ofApp::update(){
    eyes[0].update();
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
    ofRotateDeg(180, 1, 0, 0); // hide seam
    ofDrawSphere(0, 0, 0, r);
    unBindEye();
    ofRotateDeg(180, 1, 0, 0); // hide seam
    setPupil(r, 1);
    ofPopStyle();
    ofPopMatrix();
}
void ofApp::unBindEye() {
    eyes[0].getTexture().unbind(); // allow for various eyes
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
