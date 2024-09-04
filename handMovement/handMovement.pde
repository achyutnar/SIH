import processing.serial.*;
import java.io.*;
import java.net.*;

float[] fingerJoynt1 = new float[5];
float[] fingerJoynt2and3 = new float[5];
float gRotateX = 1;
float gRotateY = 0;
float gRotateZ = 3.48;
float gRotateXspeed = 0.0;
float gRotateYspeed = 0;
float gRotateZspeed = 0;
float[] finger = new float[5];
float[] fingerMin = new float[5];
float[] fingerMax = new float[5];

// Server variables
ServerSocket serverSocket;
Socket clientSocket;
BufferedReader in;
int port = 8080;

void setup() {
  //port = new Serial(this, Serial.list()[3], 38400);
  fullScreen(P3D);
  noStroke();
  
  // Set predefined angles for fingers
  fingerJoynt1[0] = radians(0);  // Little finger
  fingerJoynt1[1] = radians(0);  // Ring finger
  fingerJoynt1[2] = radians(0);  // Middle finger
  fingerJoynt1[3] = radians(00);  // Index finger
  fingerJoynt1[4] = radians(00);  // Thumb

  fingerJoynt2and3[0] = radians(45);  // Little finger
  fingerJoynt2and3[1] = radians(40);  // Ring finger
  fingerJoynt2and3[2] = radians(0);  // Middle finger
  fingerJoynt2and3[3] = radians(30);  // Index finger
  fingerJoynt2and3[4] = radians(60);  // Thumb
  
  // Start the server in a new thread to prevent blocking the draw loop
  Thread serverThread = new Thread(new Runnable() {
    public void run() {
      startServer();
    }
  });
  serverThread.start();
}

void draw() {
  background(0);
  ambientLight(50, 50, 50);
  directionalLight(255, 255, 255, .1, .17, -0.5);
  translate(width / 2, height / 2, 550);
  drawHand();
}

void drawHand() {
  pushMatrix();
  fill(#D69A50);
  rotateZ(gRotateZ);
  rotateX(gRotateX);
  box(60, 10, 50);
  
  // Fingers
  for (int i = 0; i < 5; i++) {
    pushMatrix();
    if (i == 4) { // Thumb
      translate(-25, 0, -20);
      rotateY(1.83);
    } else {
      translate(30, 0, 20 - i * 13);
    }
    
    // Draw finger with predefined angles
    drawFinger(fingerJoynt1[i], fingerJoynt2and3[i], (int)getFingerlength(i), (int)(getFingerlength(i) * 0.8));
    popMatrix();
  }
  popMatrix();
}

void drawFinger(float joynt1, float joynt2and3, int fingerLength1, int fingerLength2) {
  pushMatrix();
  rotateZ(joynt1);
  fill(#D69A50);
  sphere(6);
  translate(fingerLength1/2, 0, 0);
  box(fingerLength1, 10, 10);
  translate(fingerLength1/2, 0, 0);
  rotateZ(joynt2and3);
  sphere(6);
  translate(fingerLength2/2, 0, 0);
  box(fingerLength2, 10, 10);
  translate(fingerLength2/2, 0, 0);
  rotateZ(joynt2and3);
  sphere(6);
  translate(fingerLength2/2, 0, 0);
  box(fingerLength2, 10, 10);
  popMatrix();
}

int getFingerlength(int fingerIndex) {
  switch(fingerIndex) {
    case 0: return 13; // Little finger
    case 1: return 18; // Ring finger
    case 2: return 20; // Middle finger
    case 3: return 17; // Index finger
    case 4: return 30; // Thumb
    default: return 15;
  }
}

void keyPressed() {
  for (int i = 0; i < 5; i++) {
    fingerMin[i] = 1024;
    fingerMax[i] = 0;
  }
  println("calibrate");
}

void startServer() {
  try {
    serverSocket = new ServerSocket(port);
    println("Server listening on port " + port);

    while (true) {
      clientSocket = serverSocket.accept();
      println("Got connection from " + clientSocket.getInetAddress());
      
      in = new BufferedReader(new InputStreamReader(clientSocket.getInputStream()));
      String data = in.readLine();
      println("Received data: " + data);
      
      // Process received data (e.g., adjust angles, move fingers, etc.)
      if (data != null) {
        processData(data);
      }

      clientSocket.close();
    }
  } catch (IOException e) {
    e.printStackTrace();
  }
}

void processData(String data) {
    // Split the data into its components
    String[] parts = data.split(":");
    if (parts.length != 2) {
        return; // If the data doesn't match the expected format, skip it
    }
    
    String fingerName = parts[0];
    String[] angleStrings = parts[1].split(",");
    if (angleStrings.length != 3) {
        return; // If there aren't exactly 3 angle values, skip it
    }
    
    // Parse the angles from strings to floats
    float angle1 = radians(Float.parseFloat(angleStrings[0]));
    float angle2 = radians(Float.parseFloat(angleStrings[1]));
    float angle3 = radians(Float.parseFloat(angleStrings[2]));

    // Update the corresponding finger's angles
    if (fingerName.equals("Main")) {
        // Assuming "Main" affects the rotation of the entire hand or the base joint
        gRotateX = angle1*10 + 50;
        gRotateY = angle2*10 + 50;
        gRotateZ = angle3*10;
    } else if (fingerName.equals("Thumb")) {
        // Assuming the first joint of the thumb controls the first rotation angle
        fingerJoynt1[4] = angle1*10;
        fingerJoynt2and3[4] = angle2*10; // Assuming this controls the rest of the thumb's rotation
        // The third value might not be needed, depending on the hand model
    } else if (fingerName.equals("Middle")) {
        fingerJoynt1[2] = angle1;
        fingerJoynt2and3[2] = angle2*10;
    } else if (fingerName.equals("Point")) {
        // Assuming "Point" refers to the index finger
        fingerJoynt1[3] = angle1*10;
        fingerJoynt2and3[3] = angle2*10;
    } else if (fingerName.equals("Ring")) {
        // Add similar logic if there's data for the ring finger
        fingerJoynt1[1] = angle1+10;
        fingerJoynt2and3[1] = angle2+10;
    } else if (fingerName.equals("Little")) {
        // Add similar logic if there's data for the little finger
        fingerJoynt1[0] = angle1;
        fingerJoynt2and3[0] = angle2;
    }

    // Redraw the hand with the new angles in the next frame
    redraw();
}
