 #pragma once
#include <algorithm>
#include <random>
#include "ofMain.h"
#include "ofxAnimatableFloat.h"
#include "ofxAnimatableOfPoint.h"
#include "ofxAnimatableOfColor.h"
#include "ofxAnimatableQueue.h"
#include "ofxOpenCv.h"

#include "ofxAssimpModelLoader.h"

const int imgWidth = 640;// 320; // the motion image from the camera
const int imgHeight = 480;//240;

#define DATAPATH "runtime"

class Light : public ofLight {
public:
    void setup() {
        ofLight::setup();
        setDirectional();
        setOrientation(ofVec3f(-200.0f, 300.0f, 00.0f));
        setPosition(0, 0, 2000);

        specularcolor.setColor(ofColor::mediumVioletRed);
        specularcolor.setDuration(1.0f);
        specularcolor.setRepeatType(LOOP_BACK_AND_FORTH);
        specularcolor.setCurve(LINEAR);
        specularcolor.animateTo(ofColor::orangeRed);

        ambientcolor.setColor(ofColor::blue);
        ambientcolor.setDuration(1.5f);
        ambientcolor.setRepeatType(LOOP_BACK_AND_FORTH);
        ambientcolor.setCurve(LINEAR);
        ambientcolor.animateTo(ofColor::red);
    }
    void update() {
        specularcolor.update(1.0f / ofGetTargetFrameRate());
        ambientcolor.update(1.0f / ofGetTargetFrameRate());

        setAmbientColor(ambientcolor.getCurrentColor());
        setSpecularColor(specularcolor.getCurrentColor());
        //setAmbientColor(ofColor::mediumVioletRed);
        ///setSpecularColor(ofColor::saddleBrown);
        ////setDiffuseColor(ofColor::pink);
    }
    void setOrientation(ofVec3f rot) {
        ofVec3f xax(1, 0, 0);
        ofVec3f yax(0, 1, 0);
        ofVec3f zax(0, 0, 1);
        ofQuaternion q;
        q.makeRotate(rot.x, xax, rot.y, yax, rot.z, zax);
        ofLight::setOrientation(q);
    }
private:
    ofxAnimatableOfColor specularcolor;
    ofxAnimatableOfColor ambientcolor;

};

class Camera : public ofEasyCam {
public:
    void setup() {
        setDistance(4286); // magic number 
    }
};

class Material : public ofMaterial {
public:
    Material() {
        setup();
    }
    void setup() {
        color.setColor(ofColor::white);
        color.setDuration(1.0f);
        color.setRepeatType(LOOP_BACK_AND_FORTH);
        color.setCurve(LINEAR);
        color.animateTo(ofColor::orangeRed);// COLOR not used yet

        setShininess(120);
        setSpecularColor(ofColor::white);
        // setEmissiveColor(ofColor::black);
        setDiffuseColor(ofColor::whiteSmoke);
        setAmbientColor(ofColor::navajoWhite);
    }
    void update(){
        color.update(1.0f / ofGetTargetFrameRate());
    }
private:
    ofxAnimatableOfColor color; // not used yet

};

class Eye : public ofTexture {
public:
    Eye() {}
    Eye(const string&name) {
        setup(name);
    }
    Material material;

    void setup(const string&texName) {

        if (ofLoadImage(*this, texName)) {
            ofLogNotice("Eye") << "loaded " << texName;
        }
        else {
            ofLogError("Eye") << "not loaded " << texName;
        }
        //assimp not supported model.loadModel(objName);
    }
    void update() {
    }
    void start() {
        //color.applyCurrentColor();
        material.begin();
        bind();
    }
    void stop() {
        unbind();
        material.end();
    }
};
// always knows it rotation coordindates
class SuperSphere : public ofSpherePrimitive {
public:
    const int maxListSize = 100;

    void setup(const string&name, const string&blinkPath) {
        blinkingEnabled = true;
        eye.setup(name);
        string path = DATAPATH;
        path += "\\"+ blinkPath +".blink";
        ofDirectory dir(path);
        dir.listDir();
        blink.push_back(Eye(name));
        std::string name2;
        for (size_t i = 0; i < dir.size(); i++) {
            name2 = path + "\\";
            name2 += dir.getName(i);
            blink.push_back(Eye(name2));
        }
        blink.push_back(Eye(name2)); // last one gets skipped

        blinker.reset(0.0f);
        blinker.setCurve(LINEAR);
        blinker.setRepeatType(LOOP_BACK_AND_FORTH_ONCE);
        blinker.setDuration(0.2f);
        blinker.animateTo(blink.size() - 1);
       // blink
        setResolution(21);
        panDeg(180);
       // animateToAfterDelay
    }
    void update() {
        blinker.update(1.0f / ofGetTargetFrameRate());
        if (!blinker.isOrWillBeAnimating()) {
            blinker.reset(0.0f);
            blinker.animateToAfterDelay(blink.size() - 1, ofRandom(5));
        }
    }
    void draw() {
        int index = 0; // the non blink index
        if (blinkingEnabled) {
            index = blinker.getCurrentValue();
        }
        blink[index].start();
        ofSpherePrimitive::draw();
        blink[index].stop();
    }
    bool blinkingEnabled;
private:
    Eye eye; 
    std::vector<Eye> blink;
    ofxAnimatableFloat blinker; // blink animation
};

class ofxAnimatableQueueofVec3f {
public:
    const int maxListSize = 100;

    void setup() {
        startPlaying();
    }
    void update() {
        currentAnimation.update(1.0f / ofGetTargetFrameRate());
        if (playing) {
            if (currentAnimation.hasFinishedAnimating()) {
                if (animSteps.size() > 0) {
                    //animSteps.erase(animSteps.begin());
                    currentAnimation = animSteps.front();
                    animSteps.pop_front();
                    ofLogNotice() << "next";
                }
                else {
                    ofLogNotice() << "empty";
                }
            }
        }
    }
    void addTransition(ofxAnimatableOfPoint targetValue) {
        ofLogNotice() << "addTransition " << targetValue.getCurrentPosition();
        if (targetValue.getCurrentPosition().x == 0) {
            int i = 0;
        }
        if (animSteps.size() > maxListSize) {
            ofLogNotice() << " cap list size to maxListSize " << maxListSize;
            animSteps.pop_back(); // only keep the  most recent
        }
        animSteps.push_back(targetValue);//bugbug go to pointer
    }
    void insertTransition(ofxAnimatableOfPoint targetValue, bool forceNext) {
        if (forceNext) {
            currentAnimation = targetValue; // make this one current, drop the current one
        }
        else {
            ofLogNotice() << "insertTransition " << targetValue.getCurrentPosition();
            if (animSteps.size() > maxListSize) { // make const
                ofLogNotice() << " cap list size to maxListSize " << maxListSize;
                animSteps.pop_back(); // only keep the  most recent
            }
            animSteps.push_front(targetValue);//bugbug go to pointer
        }
    }
    bool hasFinishedAnimating() {
        return currentAnimation.hasFinishedAnimating();
    }
    ofxAnimatableOfPoint getCurrentValue() {
        return currentAnimation;
    }
    ofPoint getPoint() {
        return currentAnimation.getCurrentPosition();
    }
    void append(const ofVec3f& target) {
        ofxAnimatableOfPoint targetPoint;
        if (animSteps.size() > 0) {
            targetPoint.setPosition(getPoint());
        }
        targetPoint.animateTo(target);
        targetPoint.setDuration(1.25);
        targetPoint.setRepeatType(PLAY_ONCE);
        targetPoint.setCurve(LINEAR);
        addTransition(targetPoint);
    }
    void startPlaying() {
        playing = true;
    }

protected:
    bool playing = false;
private:
    ofxAnimatableOfPoint currentAnimation;
    std::list<ofxAnimatableOfPoint> animSteps;
};


class ContoursBuilder {
    public:
        void setup() {
            vector<ofVideoDevice> devices = video.listDevices();
            for (auto& device : devices) {
                if (device.deviceName.find("facetime") == std::string::npos) {
                    video.setDeviceID(device.id);
                    break;
                }
            }
            video.setVerbose(true);
            video.setup(imgWidth, imgHeight);
            colorImg.allocate(imgWidth, imgHeight);
            grayImage.allocate(imgWidth, imgHeight);
            grayDiff.allocate(imgWidth, imgHeight);
        }
        // return true if updated
        void update() {
            video.update();
            if (video.isFrameNew()) { // && (ofGetFrameNum() & 1) to slow things down
                                      // clear less often
                colorImg.setFromPixels(video.getPixels());
                grayImage = colorImg; // convert our color image to a grayscale image
                if (backgroundImage.bAllocated) {
                    grayDiff.absDiff(backgroundImage, grayImage);
                }
                backgroundImage = grayImage; // only track new items -- so eye moves when objects move
                grayDiff.threshold(50); // turn any pixels above 30 white, and below 100 black
                if (!contourFinder.findContours(grayDiff, 5, (imgWidth*imgHeight), 128, false, true)) {
                    contourFinder.blobs.clear(); // removes echo but does it make things draw too fast?
                }
                grayImage.blurGaussian(3);
                grayImage.threshold(50);
                if (!contourDrawer.findContours(grayImage, 5, (imgWidth*imgHeight), 128, true)) {
                    contourDrawer.blobs.clear(); 
                }
            }
        }
        void draw(float cxScreen, float cyScreen) {
            ofPushStyle();
            ofPushMatrix();
            ofNoFill();
            ofSetLineWidth(1);// ofRandom(1, 5));
            for (auto& blob : contourDrawer.blobs) {
                ofPolyline line;
                for (int i = 0; i < blob.nPts; i++) {
                    line.addVertex((imgWidth - blob.pts[i].x), blob.pts[i].y);
                }
                line.close();
                line.scale(cxScreen / imgWidth, cyScreen / imgHeight);
                line.draw();
            }
            ofSetLineWidth(5);// ofRandom(1, 5));
            if (contourFinder.blobs.size() > 0){
                for (auto& blob : contourFinder.blobs) {
                    ofPolyline line;
                    for (int i = 0; i < blob.nPts; i++) {
                        line.addVertex((imgWidth - blob.pts[i].x), blob.pts[i].y);
                    }
                    line.close();
                    line.scale(cxScreen / imgWidth, cyScreen / imgHeight);
                    line.draw();
                    //ofDrawRectangle(blob.boundingRect.x, blob.boundingRect.y, blob.boundingRect.width, blob.boundingRect.height);
                }
            }
            ofPopMatrix();
            ofPopStyle();
        }

        ofxCvContourFinder contourFinder;
        ofxCvContourFinder contourDrawer;
    private:
        ofVideoGrabber video;
        ofxCvColorImage colorImg;
        ofxCvGrayscaleImage grayImage, backgroundImage, grayDiff;
    };

class ImageAnimator {
    public:
        void setup() {
            ofSetFrameRate(60.0f);
            buildX();
            buildY();
            animatorIndex.reset(0.0f);
            animatorIndex.setDuration(1.0f);
            animatorIndex.setRepeatType(LOOP_BACK_AND_FORTH);
            animatorIndex.setCurve(LINEAR);

            path.setup();
            rotator.setup();

            contours.setup();

            string path = DATAPATH;
            ofDirectory allEyes(path);
            allEyes.allowExt("png");
            allEyes.allowExt("jpg");
            allEyes.listDir();
            size_t i = 0;
            for (; i < allEyes.size(); i++) {
                add(allEyes.getPath(i), allEyes.getName(i));
            }
            ofDirectory allSounds(path);
            allSounds.allowExt("wav");
            allSounds.allowExt("mp3");
            allSounds.listDir();
            for (i = 0; i < allSounds.size(); i++) {
                ofSoundPlayer sound;
                sound.load(allSounds.getPath(i));
                mySounds.push_back(sound);
            }

            startPlaying();

        }
        //http://www.findsounds.com/ISAPI/search.dll?keywords=cat
        void sounds() {
            auto rng = std::default_random_engine{};
            std::shuffle(std::begin(mySounds), std::end(mySounds), rng);

            for (ofSoundPlayer&player : mySounds) {
                player.setVolume(ofRandom(2.2f));
                player.setPosition(ofRandom(1.0f));
                player.setMultiPlay(true);
                int end = (int)ofRandom(1, 10);
                for (int i = 0; i < end; ++i) {
                    player.play();
                }
                // restore for next time
                player.setMultiPlay(true);
                player.setPosition(0);
                player.setVolume(1.0);
            }
       } 
        float calc(float val, float start, float end, float len) {
            float result = start;
            float factor = 0.10f;
            float inc = fabsf(start - end)/10;
            for (; true; result += inc, factor += 0.10f) {
                if (val< len*factor) {
                    break;
                }
            }
            ofLogNotice() << "result " << result << " source " << val << " of " << len;
            return result;
        }
        void circle() {
            ofxAnimatableOfPoint point;
            point.setPosition(currentLocation);
            point.setDuration(1.20f);
            point.animateTo(ofVec3f(1000, 1000, 10));
            path.addTransition(point);
            point.animateTo(ofVec3f(1000, 2000,200));
            path.addTransition(point);
            point.animateTo(ofVec3f(2000, 2000, -200));
            path.addTransition(point);
            point.animateTo(ofVec3f(1000, 1000,3000));
            path.addTransition(point);
            point.animateTo(ofVec3f(00, 00));
            path.addTransition(point);

        }
        void update() {
            for (SuperSphere&eye : eyes) {
                eye.update();
            }
            animatorIndex.update(1.0f / ofGetTargetFrameRate());
            path.update();
            rotator.update();
            contours.update();

            /* special effects
            if (path.hasFinishedAnimating()) {
               circle();
            }
            if (rotator.hasFinishedAnimating()) {
                ofxAnimatableOfPoint point;
                point.setPosition(currentRotation);
                point.animateTo(ofVec3f(ofRandom(90.0f), ofRandom(90.0f)));
                rotator.addTransition(point);
            }
            */

            // track motion
            float max = 0.0f;
            if (contours.contourFinder.blobs.size() > 0) {
                glm::vec3 target = currentRotation;
                ofDefaultVec3 centroid;
                // find max size
                for (auto& blob : contours.contourFinder.blobs) {
                    if (blob.area > max && blob.boundingRect.x > 1 && blob.boundingRect.y > 1) {  //x,y 1,1 is some sort of strange case
                        max = blob.area;
                        centroid = blob.centroid;
                        break; // first is max
                    }
                }
                if (max > 100) { // fine tune on site
                    int w = imgWidth; // camera size not screen size
                    int h = imgHeight;

                    double x = (centroid.x / imgWidth)*100.0f; // make it a percent
                    double y = (centroid.y / imgHeight)*100.0f; // make it a percent
                    std::map<std::pair<int, int>, int> thingsToDo;
                    thingsToDo.insert(std::make_pair(std::make_pair(1,1), 1)); // upper left cat noise

                    int xAction = 0; // none
                    int yAction = 0; // none
                    if (mapX.find(std::make_pair(xAction, yAction)) != mapX.end()) {
                    }
                    for (auto& row : mapX) {
                        if (x >= row.first.first && x <= row.first.second){
                            target.y = row.second.rotation;
                            xAction = row.second.action;
                            break;
                        }
                    }
                    for (auto& row : mapY) {
                        if (y >= row.first.first && y <= row.first.second) {
                            target.x = row.second.rotation;
                            yAction = row.second.action;
                            break;
                        }
                    }
                    if (thingsToDo.find(std::make_pair(xAction, yAction)) != thingsToDo.end()) {
                        int fun = thingsToDo[std::make_pair(xAction, yAction)];
                        if (fun == 1) {
                            sounds(); // done one for eyese and one for both
                        }
                    }

                }
                // if any data 
                if (max > 10) {
                    ofLogNotice() << "insert targert" << target;
                    currentRotation = target;
                }
                else {
                   // no new data so home the eye (?should we add a time?)
                    currentRotation.set(0.0L, 0.0L, 0.0L);
                }
                std::stringstream ss;
                ss << max;
                ofSetWindowTitle(ss.str());

                /*
                if (found && 0) {
                    ofxAnimatableOfPoint point;
                    // get the current point -- smooth things out
                    point.setPosition(currentRotation);
                    point.setCurve(LINEAR);
                    point.setRepeatType(PLAY_ONCE);
                    point.setDuration(0.2f);
                    point.animateTo(target);
                    rotator.insertTransition(point, true);
                }
                */

            }
        }
        void windowResized(int w, int h) {
            for (SuperSphere&eye : eyes) {
                eye.setRadius(std::min(w, h));
            }
        }
        void draw() {
            getCurrentEyeRef().blinkingEnabled = false; // only blink when eye is not doing interesting things bugbug fix blinking
            // move all eyes so when they switch things are current
            if (!path.hasFinishedAnimating()) {
               //currentLocation = path.getPoint();
              // getCurrentEyeRef().blinkingEnabled = false;
 // do location later, its just for special effectgs               getCurrentEyeRef().setPosition(currentLocation);
            }


           // roate current eye as needed
           //if (!rotator.hasFinishedAnimating()) {
           //    currentRotation = rotator.getPoint();
           //    getCurrentEyeRef().blinkingEnabled = false;
          // }
           rotate(currentRotation);
           getCurrentEyeRef().draw();
        }
        void startPlaying() {
            animatorIndex.animateTo(eyes.size()-1);
            sounds();
        }
        SuperSphere& getCurrentEyeRef() {
            return eyes[(int)animatorIndex.getCurrentValue()];
        }
        void add(const std::string &name, const std::string &root) {
                std::string::size_type pos = root.find('.');
                std::string blinkPath;
                if (pos != std::string::npos) {
                    blinkPath = root.substr(0, pos);
                }
                else {
                    blinkPath = root;
                }
            eyes.push_back(SuperSphere());
            eyes[eyes.size()-1].setup(name, blinkPath);
        }
        ContoursBuilder contours;

    private:
        void buildX() {
            float percent = 0.0f;// location as a percent of screen size
            float r = 30.0f; // rotation
            float incPercent = 5.0f;
            float incRotaion = ((r * 2) / (100.0f / incPercent - 1));
            for (int i = 0; percent < 100.0f; ++i, percent += incPercent, r -= incRotaion) {
                mapX.insert(std::make_pair(std::make_pair(percent, percent + incPercent), Map(r, i + 1)));
            }
        }
        void buildY() {
            float percent = 0.0f;// location as a percent of screen size
            float r = -20.0f; // rotation
            float incPercent = 5.0f;
            float incRotaion = ((r * 2) / (100.0f / incPercent - 1));
            for (int i = 0; percent < 100.0f; ++i, percent += incPercent, r -= incRotaion) {
                mapY.insert(std::make_pair(std::make_pair(percent, percent + incPercent), Map(r, i + 1)));
            }
        }

        std::vector<ofSoundPlayer> mySounds;
        void rotate(const ofVec3f& target) {
            std::stringstream ss;
            ss << target;
            //ofSetWindowTitle(ss.str());
           // if (fabs(target.x) > 16.0) draw less
            if (target.x || target.y || target.z) {
                ofLogNotice() << "rotate to targert" << target;
                ofRotateDeg(target.x, 1.0f, 0.0f, 0.0f);
                ofRotateDeg(target.y, 0.0f, 1.0f, 0.0f);
                ofRotateDeg(target.z, 0.0f, 0.0f, 1.0f);
            }
        }
        ofxAnimatableFloat animatorIndex;
        ofxAnimatableQueueofVec3f rotator;
        ofxAnimatableQueueofVec3f path; // path of image
        std::vector<SuperSphere> eyes;
        ofVec3f currentLocation;
        ofVec3f currentRotation;
        class Map {
        public:
            Map(float r, int a) {
                rotation = r;
                action = a;
            }
            float rotation;
            int action; // things like have the  cat noise when hit
        };
        std::map<std::pair<float, float>, Map> mapX;
        std::map<std::pair<float, float>, Map> mapY;

    };



class ofApp : public ofBaseApp{

	public:
        Light	light;
        Camera camera;
        ImageAnimator eyeAnimator;
        ofMesh m;
        void setup() {
            //ofEnableSeparateSpecularLight();
            ofSetWindowShape(ofGetScreenWidth(), ofGetScreenHeight());
            ofSetBackgroundColor(ofColor::black);
            ofSetColor(ofColor::white);
            ofSetLogLevel(OF_LOG_VERBOSE);
            ofLogToConsole();
            //ofEnableLighting();
            ofEnableDepthTest();
            ofSetVerticalSync(true);
            ofDisableArbTex();
            //ofSetSmoothLighting(true);
            ofDisableAlphaBlending();
            camera.setup();
           // camera.lookAt(eyeAnimator.sphere);
            eyeAnimator.setup();
            light.setup();
        }
        //--------------------------------------------------------------
        void update() {
            eyeAnimator.update();
            light.update();
            //too much only use when needed camera.setDistance(eyeAnimator.camera.getCurrentValue());
            // light.setPosition(0, 0, eyeAnimator.camera.getCurrentValue() - 1500);
            // debug helper
            std::stringstream ss;
            //ss << camera.getDistance();
           // ofSetWindowTitle(ss.str());
        }
        
        //--------------------------------------------------------------
        void draw() {
            ofPushStyle();
            light.enable();
            camera.begin();
            ofPushMatrix();
            ofTranslate((ofGetWidth() / 2) - eyeAnimator.getCurrentEyeRef().getRadius(), ofGetHeight() / 2 - eyeAnimator.getCurrentEyeRef().getRadius()/2, 0);
            eyeAnimator.draw();
            ofPopMatrix();
            camera.end();
            eyeAnimator.contours.draw(ofGetScreenWidth(), ofGetScreenHeight());
            light.disable();
            ofPopStyle();
            ofPopMatrix();
        }
        //--------------------------------------------------------------
        void windowResized(int w, int h) {
            eyeAnimator.windowResized(w, h);
        }

private:
};
