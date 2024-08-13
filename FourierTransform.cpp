#define _USE_MATH_DEFINES
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <iostream>
#include <cmath>
#include <vector>
#include <math.h>
#include <thread>
#include <mutex>
#include <chrono>
#include <algorithm> 
#include <iostream> 
#include <vector> 


using namespace std;

using namespace sf;
using namespace std;
using namespace std::chrono;


const int frameheight = 1000;
const int framewidth = 1800;
const float pointrad = 3;
const int numberofcircles = 25;
RenderWindow window(sf::VideoMode(framewidth, frameheight), "PLINKO");

class circle {
public:
    float radius;
    float phase;
    float freq;


    circle( float rad, float fre,float pha) :
         radius(rad), freq(fre), phase(pha) {}
};

vector<circle> dft(vector<float> vals) {
    vector<circle> ans = {};
    
    const int N = vals.size();
    for (int k = 0; k < N; k++) {
        float real = 0;
        float imaginary = 0;
        for (int n = 0; n < N; n++) {
            const float phi = (M_PI * 2 * k * n) / N;
            real += vals[n] * cos(phi);
            imaginary -= vals[n] * sin(phi);
        }


        real /= N;
        imaginary /= N;
        float frequency = k;
        float phase = atan2(imaginary, real);
        float amplitude = sqrt(imaginary * imaginary + real * real);
        ans.emplace_back(circle(amplitude, frequency, phase));
    }
    sort(ans.begin(), ans.end(), [](const circle& a, const circle& b) {
        return a.radius > b.radius;});
    return ans;
}



Vector2f epiCycles(float x, float y, float rotation, vector<circle> fourier, float time) {
    const float radiusmax = fourier[0].radius;
    for (int i = 0; i < fourier.size(); i++) {
        float prevx = x;
        float prevy = y;
        float freq = fourier[i].freq;
        float radius = fourier[i].radius;
        float phase = fourier[i].phase;
        x += radius * cos(freq * time + phase + rotation);
        y += radius * sin(freq * time + phase + rotation);


        
        CircleShape circle(radius);
        circle.setFillColor(Color(0, 0, 0, 0));
        circle.setOutlineColor(Color(255, 255, 255, 255));
        circle.setOutlineThickness(1);
        circle.setPosition(Vector2f(prevx, prevy) - Vector2f(radius, radius));
        window.draw(circle);


        VertexArray lines(LinesStrip, 2);
        lines[0] = Vector2f(prevx, prevy);
        lines[1] = Vector2f(x, y);
        window.draw(lines);
        
    }
    return Vector2f(x, y);
}
double mapping(double value, double start1, double stop1, double start2, double stop2) {
    return start2 + (stop2 - start2) * ((value - start1) / (stop1 - start1));
}

int main()
{
    
    Event event;
    Clock clock;
    vector<CircleShape> wave;


    //setting the drawing
    vector<float> x;
    vector<float> y;


    // initial drawing(circle)
    for (int i = 0; i < 100; i++) {
        double angle = mapping(i, 0, 100, 0, 3.141592 * 2);
        x.emplace_back(100 * cos(angle));
    }
    for (int i = 0; i < 100; i++) {
        float angle = mapping(i, 0, 100, 0, 3.141592 * 2);
        y.emplace_back(100 * sin(angle));
    }
    vector<circle>fourierX = dft(x);
    vector<circle>fourierY = dft(y);
    //processing the drawing
    


    vector<Vector2f> currdrawaing;
    bool lock_click = false;
    float time = 0;
    window.setFramerateLimit(60);

    RectangleShape linemiddle(Vector2f(5, frameheight));
    
    linemiddle.setPosition(Vector2f(framewidth/2, 0));
    linemiddle.setFillColor(Color(255, 0, 0, 255));

    Font font;
    if (!font.loadFromFile("C:\\Windows\\Fonts\\impact.ttf")) {
        cerr << "Error loading font\n";
        return -1;
    }
    Text draw_text;
    draw_text.setFont(font);
    draw_text.setString("DRAW HERE");
    draw_text.setPosition(Vector2f(350, 10));

    while (window.isOpen()) {



        window.clear();
        window.draw(linemiddle);
        window.draw(draw_text);
        const float dt = (M_PI * 2) / fourierY.size();
        time += dt;

        
        while (window.pollEvent(event)) {
            if (event.type == Event::Closed) {
                window.close();
            }
            if (event.type == Event::MouseButtonPressed and event.mouseButton.button == Mouse::Left && lock_click != true) //Mouse button Pressed
            {
                x = {};
                y = {};
                currdrawaing = {};

                lock_click = true;
            }
            if (event.type == Event::MouseButtonReleased and event.mouseButton.button == Mouse::Left) //Mouse button Released now.
            {
                lock_click = false; //unlock when the button has been released.
                fourierX = dft(x);
                fourierY = dft(y);
                wave = {};

            }
        }
    
        // Things that should always be calculated
        
        Vector2f vx = epiCycles(framewidth / 2 + framewidth / 4, 100, 0, fourierX, time);
        Vector2f vy = epiCycles(framewidth / 2 + 100, frameheight / 2, M_PI / 2, fourierY, time);
        CircleShape point(3);
        point.setPosition(Vector2f(vx.x, vy.y) - Vector2f(3,3));
        point.setFillColor(Color(255,255,255,255));
        window.draw(point);


        wave.emplace_back(point);

        VertexArray drawing(LinesStrip,currdrawaing.size());
        for (int x = 0; x < currdrawaing.size();x++) {
            drawing[x] = currdrawaing[x];
            
        }
        window.draw(drawing);

        if (lock_click == false){
            VertexArray leftcenter(LineStrip, 2);
            leftcenter[0] = Vector2f(vx.x, vx.y);
            leftcenter[1] = Vector2f(vx.x, vy.y);
            VertexArray topecenter(LineStrip, 2);
            topecenter[0] = Vector2f(vy.x, vy.y);
            topecenter[1] = Vector2f(vx.x, vy.y);

            window.draw(leftcenter);
            window.draw(topecenter);


            VertexArray waveline(LineStrip, wave.size());
            if (time > M_PI * 2) {
                wave.clear();
                time = 0;
            }
            for (int i = 0; i < wave.size(); i++) {
                waveline[i] = Vector2f(wave[i].getPosition().x, wave[i].getPosition().y);
                waveline[i].color = Color(0, 255, 0, 255);
            }
            window.draw(waveline);

        }
        else {
            Vector2f position = Vector2f(Mouse::getPosition(window));
            if (position.x < framewidth / 2 and position.x > 0) {
                currdrawaing.emplace_back(position);
                x.emplace_back(position.x -framewidth/4);
                y.emplace_back(position.y - frameheight/2);
            }


        }

        window.display();
    }


    return 0;
}
