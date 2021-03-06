#pragma once
#include <deque>
#include "ofMain.h"
#include "ofxOpenCv.h"

// add this in, some of times paitinghs and then the interaction 
//https://github.com/bakercp/ofxIpVideoGrabber

const int cx = 3840; // screen size
const int cy = 2160;
const int cxTracer = 640;
const int cyTracer = 480;

//3,840 by 2,160 ir 1024 by 768
class Camera : public ofEasyCam {
public:
    // nice drawing tool
    void drawit() {
        float time = ofGetElapsedTimef();
        float longitude = 10 * time;
        float latitude = 10 * sin(time*0.8);
        float radius = 800 + 50 * sin(time*0.4);
        orbitDeg(longitude, latitude, radius, ofPoint(0, 0, 0));
    }
    //headTrackedCamera.begin();
    //headTrackedCamera.panDeg(0.5);
    //headTrackedCamera.truck(1.0);
    //headTrackedCamera.dolly(-1.0);
    //headTrackedCamera.boom(1.0);
    //headTrackedCamera.tiltDeg(5);

};

class Timer {
public:
    uint64_t t1 = 0;
    uint64_t duration = 60; // 2 sec or so
    void start() {
        t1 = ofGetFrameNum();
    }
    bool current() {
        return duration > ofGetFrameNum() - t1;
    }
};
class MotionData {
public:
    MotionData() {}
    MotionData(glm::vec2&r, ofRectangle &rec, float f, int index=0) {
        rotation = r;
        rect = rec;
        freq = f;
        this->index = index;
    }
    void draw(int scaleX, int scaleY, bool fill = false) const {
        ofRectangle rectToScale = rect;
        rectToScale.scale(scaleX, scaleY);
        if (fill) {
            ofFill();
        }
        else {
            ofNoFill();
        }
        ofDrawRectangle(rectToScale.x*scaleX, rectToScale.y*scaleY, rectToScale.width, rectToScale.height);
    }
    bool found(int x, int y) {
        return rect.inside(x, y);
    }
    glm::vec2 rotation;
    ofRectangle rect; // target
    int index = 0;
    float freq = 0.0;
};

class myBlob : public ofxCvBlob {
public:
    void set(ofxCvBlob& b) {
        blob = b;
        t.duration = 5;
        t.start();
    }
    Timer t;
    ofxCvBlob blob;
};
class ArtImage : public ofImage {
public:
    ArtImage(int cx, int cy, const std::string& path) {
        load(path);
        resize(cx, cy);
    }
    void set(int scaleX, int scaleY) {
        this->scaleX = scaleX;
        this->scaleY = scaleY;
        t.start();
    }
    void TimedDraw() const {
        ofRectangle rectToScale = motion.rect;
        rectToScale.scale(scaleX, scaleY);
        ofEnableAlphaBlending();
        draw(rectToScale.x*scaleX, rectToScale.y*scaleY, rectToScale.width, rectToScale.height);
        ofDisableAlphaBlending();
    }

    Timer t;
    int scaleX = 1, scaleY = 1;
    MotionData motion;
};


class ManagedEye : public ofImage {
public:
    MotionData data;
    struct Range {
        float end;
    };
    struct Ranges {
        Range X, Y;
    };
    void setup() {
        load(path);
        // size of eye relative to screen
        resize(ofGetWidth()*0.57, ofGetHeight()*0.57); // size related to cat
        memset(&rotation, 0, sizeof(rotation));
    }
    void set(const MotionData&motion) {
        rotation.X.end = motion.rotation.x;
        rotation.Y.end = motion.rotation.y;
    }
    void update(){
    }
    void draw() { // draw what is in vector
        ofEnableAlphaBlending();
        ofPushMatrix();
        //ofTranslate(getWidth() / 2, getHeight() / 2, 0);//move pivot to centre
        ofTranslate(getWidth() / 2, getHeight() / 2, 0);
        //rotation.X.end
        if (rotation.X.end) {
            int i = 0;
        }
        ofRotateXDeg(rotation.X.end);
        ofRotateYDeg(rotation.Y.end);
        ofPushMatrix();
        ofTranslate(-getWidth() / 2, -getHeight() / 2, 0);//move back by the centre offset
        ofImage::draw(ofGetWidth() / 2 - (getWidth() / 2), ofGetHeight() / 2 - (getHeight() / 3));  
        ofPopMatrix();
        ofPopMatrix();
        ofDisableAlphaBlending();
    }
private:
    Ranges rotation; // start to finish
    const std::string path = "eye3.jpg"; //bugbug hard coded path... ? maybe derived classes change this?
};

class Contours {
public:
    std::vector<MotionData> motionMap;

    const MotionData& find(int x, int y) {
        for (auto& motion : motionMap) {
            if (motion.found(x, y)) {
                return motion;
            }
        }
        return motionMap[0];// default to empty data, no movement needed
    }
    //v.x - motion x, v.y motion y, v.w - width, v.z - length
    void add(const ofRectangle& grid, float freq, int index) { // private
        float w = grid.getWidth();
        float h = grid.getHeight();
        float row = motionMap.size()/4;
        motionMap.push_back(MotionData(glm::vec2(grid.getX(), grid.getY()), ofRectangle(w*0, row*h, w, h), freq - 250.0, index)); //ex: row 1, col 1
        motionMap.push_back(MotionData(glm::vec2(grid.getX() + 5.0, grid.getY()), ofRectangle(w*1, row*h, w, h), freq - 100.0, index + 1));// row 1, col 2
        motionMap.push_back(MotionData(glm::vec2(grid.getX() + 7.0, grid.getY()), ofRectangle(w * 2, row*h, w, h), freq, index + 2)), index+2;// row 1, col 3
        motionMap.push_back(MotionData(glm::vec2(grid.getX() + 9.0, grid.getY()), ofRectangle(w * 3, row*h, w, h), freq+1000, index + 3));// row 1, col 4
    }
    void setup() {
        //video.setVerbose(true);
        vector<ofVideoDevice> devices= video.listDevices();
        for (auto& device : devices) {
            if (device.deviceName.find("FaceTime") == std::string::npos) {
                video.setDeviceID(device.id);
                break;
            }
        }                                                                                   
        float w = imgWidth / 4;
        float h = imgHeight / 4;
        // 0-360
        add(ofRectangle(-12.0, 10.0, w, h), 250, 0); // col 0, % of 180
        add(ofRectangle(-10.0, 5.0, w, h), 500, 4); // col 1
        add(ofRectangle(10, -7.0, w, h), 1000, 8); // col 2
        add(ofRectangle(12, -10.0, w, h), 2000,12); // col 3

        eye.setup();
        art.push_back(ArtImage(cx / 4, cy / 4, "tom1.jpg"));
        art.push_back(ArtImage(cx / 4, cy / 4, "tom2.jpg"));
        art.push_back(ArtImage(cx / 4, cy / 4, "tom3.jpg"));
        art.push_back(ArtImage(cx / 4, cy / 4, "tom4.jpg"));

        art.push_back(ArtImage(cx / 4, cy / 4, "tom5.jpg"));
        art.push_back(ArtImage(cx / 4, cy / 4, "tom6.jpg"));
        art.push_back(ArtImage(cx / 4, cy / 4, "tom7.jpg"));
        art.push_back(ArtImage(cx / 4, cy / 4, "tom8.jpg"));

        art.push_back(ArtImage(cx / 4, cy / 4, "tom9.jpg"));
        art.push_back(ArtImage(cx / 4, cy / 4, "tom10.jpg"));
        art.push_back(ArtImage(cx / 4, cy / 4, "tom11.jpg"));
        art.push_back(ArtImage(cx / 4, cy / 4, "tom12.jpg"));

        art.push_back(ArtImage(cx / 4, cy / 4, "tom13.jpg"));
        art.push_back(ArtImage(cx / 4, cy / 4, "tom14.jpg"));
        art.push_back(ArtImage(cx / 4, cy / 4, "tom14.jpg"));
        art.push_back(ArtImage(cx / 4, cy / 4, "tom14.jpg"));

        video.setVerbose(true);
        video.setup(imgWidth, imgHeight);
        colorImg.allocate(imgWidth, imgHeight);
        grayImage.allocate(imgWidth, imgHeight);
        grayDiff.allocate(imgWidth, imgHeight);
        ofEnableLighting();
        ofEnableSeparateSpecularLight();
        light.enable();
        light.setAmbientColor(ofColor::paleVioletRed);
        light.setAreaLight(ofGetWidth(), ofGetHeight());
        ofEnableSmoothing();

    }
    void update() {
        video.update();
        eye.update();
        for (auto& a : art) {
            a.update();
        }
        if (video.isFrameNew()) { // && (ofGetFrameNum() & 1) to slow things down
            // clear less often
            colorImg.setFromPixels(video.getPixels());
            grayImage = colorImg; // convert our color image to a grayscale image
            //grayImage.blurHeavily();
            if (backgroundImage.bAllocated) {
                grayDiff.absDiff(backgroundImage, grayImage);
            }
            backgroundImage = grayImage; // only track new items -- so eye moves when objects move
            grayDiff.threshold(50); // turn any pixels above 30 white, and below 100 black
            contourFinder.findContours(grayDiff, 5, (imgWidth*imgHeight), 128, false, true);
        }
    }
    void draw(const ofxCvBlob& blob){
        ofPolyline line;
        //video.draw(video.getWidth(),0,-video.getWidth(),video.getHeight());
        for (int i = 0; i < blob.nPts; i++) {
            line.addVertex(ofPoint(imgWidth -blob.pts[i].x, blob.pts[i].y));
            //ofVertex(blob.pts[i].x, blob.pts[i].y);
        }
        line.close();
        line.scale(cx/imgWidth, cy/imgHeight);
        line.draw();
        //ofSetHexColor(0xff0099);
        //ofDrawRectangle(x + blob.boundingRect.x, y + blob.boundingRect.y, blob.boundingRect.width, blob.boundingRect.height);
    }
    void draw() {
        //if (thresholdImage.bAllocated) {
            //ofSetColor(ofColor::blue, 20);
            //thresholdImage.draw(x, y);
       // }
        // what does this do? contourFinder.draw();
        //ofSetColor(ofColor::blue, 20);
        //grayImage.draw(0, 0);
        //ofEnableAlphaBlending();
        ofPushStyle();
        ofNoFill();
        ofSetLineWidth(1);// ofRandom(1, 5));
        light.setAmbientColor(ofColor::paleVioletRed);
        bool first = true;
        for (auto& blob : contourFinder.blobs) {
            if (first) {
                ofNoFill();
               // for (auto& motion : motionMap) {
               ///     motion.draw(cx / imgWidth, cy / imgHeight);
//}
                MotionData data = find(imgWidth - blob.centroid.x, blob.centroid.y);
                art[data.index].motion = data;
                art[data.index].set(cx / imgWidth, cy / imgHeight);
                eye.set(art[data.index].motion);
                //motion.draw((cx / imgWidth), cy / imgHeight, true);
                artSelected.push_back(art[data.index]);
                first = false; // largest blob is first
                freq = data.freq; // give it more range, left and right etc once working
            }
            myBlob b;
            b.set(blob);
            myBlobs.push_back(b);
        }
        if (contourFinder.blobs.size() == 0) {
            light.setAmbientColor(ofColor::orangeRed);
            eye.draw();
            freq = 0;
            //ofSetColor(ofColor(255, 215, 0, 255));
        }
        light.setAmbientColor(ofColor::paleGoldenRod);
        //ofSetColor(ofColor::paleGoldenRod);
        for (auto art = artSelected.begin(); art != artSelected.end(); ) {
            if ((*art).t.current()) {
                (*art).TimedDraw();
                ++art;
            }
            else {
                art = artSelected.erase(art);
            }
        }
        light.setAmbientColor(ofColor::white);
        ofSetColor(ofColor(ofRandom(100, 255), ofRandom(100, 255), ofRandom(100, 255)));
        for (auto blob = myBlobs.begin(); blob != myBlobs.end(); ) {
            if ((*blob).t.current()) {
                //light.setAmbientColor(ofColor(ofRandom(255), ofRandom(255), ofRandom(255)));
                draw((*blob).blob);
                ++blob;
            }
            else {
                blob = myBlobs.erase(blob);
            }
        }

        ofPopStyle();
    }
    ofxCvContourFinder contourFinder;
    ofVideoGrabber video;
    ofxCvColorImage colorImg;
    ofxCvGrayscaleImage grayImage, backgroundImage, grayDiff;
    int imgWidth = cxTracer;// 320; // the motion image
    int imgHeight = cyTracer;//240;
    ManagedEye eye;
    ofLight light;
    Camera cam;
    std::vector<myBlob> myBlobs;
    std::vector<ArtImage> art;
    std::vector<ArtImage> artSelected;
    float freq = 1000;
};

// get all logic into one place
class ElectricCat  {
public:
    void setup() {
        ofSetColor(ofColor::white);
        ofSetBackgroundColor(ofColor::black);
        ofSetFrameRate(30);
        countours.setup();
        ofSetVerticalSync(true);
        ofSoundStreamSetup(1, 0); // mono output

        // Set the video grabber to the ofxPS3EyeGrabber.
    }
    void update() {
        countours.update();
    }
    void draw() {
        ofEnableDepthTest();
       countours.draw();
       //cam.end();
        // put on the sun shades
    }
    Contours countours;

};

class ofApp : public ofBaseApp {

public:
    ElectricCat art;

    ofSoundStream soundStream;

    float 	pan;
    int		sampleRate;
    bool 	bNoise;
    float 	volume;

    vector <float> lAudio;
    vector <float> rAudio;

    //------------------- for the simple sine wave synthesis
    float 	targetFrequency;
    float 	phase;
    float 	phaseAdder;
    float 	phaseAdderTarget;
    void setup() {
        art.setup();
        int bufferSize = 512;
        sampleRate = 44100;
        phase = 0;
        phaseAdder = 0.0f;
        phaseAdderTarget = 0.0f;
        volume = 0.1f;
        bNoise = false;

        lAudio.assign(bufferSize, 0.0);
        rAudio.assign(bufferSize, 0.0);

        soundStream.printDeviceList();

        ofSoundStreamSettings settings;
        settings.setOutListener(this);
        settings.sampleRate = sampleRate;
        settings.numOutputChannels = 2;
        settings.numInputChannels = 0;
        settings.bufferSize = bufferSize;
        soundStream.setup(settings);
    }

    void update() {
        art.update();
    }

    void draw() {
        art.draw();
    }
    void keyPressed(int key) {
        if (key == 'k') {
            ofToggleFullscreen();
        }
        else if (key == 'f') {
            ofToggleFullscreen();
        }
        else if (key == 's') {
            soundStream.start();
        }
        else if (key == 'e') {
            soundStream.stop();
        }
        else if (key == '-' || key == '_') {
            volume -= 0.05;
            volume = MAX(volume, 0);
        }
        else if (key == '+' || key == '=') {
            volume += 0.05;
            volume = MIN(volume, 1);
        }

    }
    void mouseMoved(int x, int y) {
        int width = ofGetWidth();
        pan = (float)x / (float)width;
        float height = (float)ofGetHeight();
        float heightPct = ((height - y) / height);
        targetFrequency = 2000.0f * heightPct;
        phaseAdderTarget = (targetFrequency / (float)sampleRate) * TWO_PI;
    }
    void audioOut(ofSoundBuffer & buffer) {
        //pan = 0.5f;
        float leftScale = 1 - pan;
        float rightScale = pan;
        if (art.countours.freq) {
            phaseAdderTarget = ((art.countours.freq*10) / (float)sampleRate) * TWO_PI;
        }
        else {
            return;
        }

        // sin (n) seems to have trouble when n is very large, so we
        // keep phase in the range of 0-TWO_PI like this:
        while (phase > TWO_PI) {
            phase -= TWO_PI;
        }

        if (bNoise == true) {
            // ---------------------- noise --------------
            for (size_t i = 0; i < buffer.getNumFrames(); i++) {
                lAudio[i] = buffer[i*buffer.getNumChannels()] = ofRandom(0, 1) * volume * leftScale;
               // rAudio[i] = buffer[i*buffer.getNumChannels() + 1] = ofRandom(0, 1) * volume * rightScale;
            }
        }
        else {
            phaseAdder = 0.95f * phaseAdder + 0.05f * phaseAdderTarget;
            for (size_t i = 0; i < buffer.getNumFrames(); i++) {
                phase += phaseAdder;
                float sample = sin(phase);
                lAudio[i] = buffer[i*buffer.getNumChannels()] = sample * volume * leftScale;
               // rAudio[i] = buffer[i*buffer.getNumChannels() + 1] = sample * volume * rightScale;
            }
        }

    }

    //--------------------------------------------------------------
    void mouseDragged(int x, int y, int button) {
        int width = ofGetWidth();
        pan = (float)x / (float)width;
    }

    //--------------------------------------------------------------
    void mousePressed(int x, int y, int button) {
        bNoise = true;
    }


    //--------------------------------------------------------------
    void mouseReleased(int x, int y, int button) {
        bNoise = false;
    }


};


/* reference
void rotateY(ofImage &image, float degrees, int x, int y) {
ofPushMatrix();
ofTranslate(image.getWidth() / 2, image.getHeight() / 2, 0);//move pivot to centre
ofRotateYDeg(degrees);
ofPushMatrix();
ofTranslate(-image.getWidth() / 2, -image.getHeight() / 2, 0);//move back by the centre offset
image.draw(x, y);
ofPopMatrix();
ofPopMatrix();
}

*/
