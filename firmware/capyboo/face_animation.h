
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include "animation_bitmap.h"
extern Adafruit_SH1106G display;
int current_frame = 0;

void playWakeupAnimation() {
	 static const int wakeup_bitmap_allArray_LEN = 25;
	 static const unsigned char* wakeup_bitmap_allArray[25] = {
		 wakeup_bitmap_00,
		 wakeup_bitmap_01,
		 wakeup_bitmap_02,
		 wakeup_bitmap_03,
		 wakeup_bitmap_04,
		 wakeup_bitmap_05,
		 wakeup_bitmap_06,
		 wakeup_bitmap_07,
		 wakeup_bitmap_08,
		 wakeup_bitmap_09,
		 wakeup_bitmap_10,
		 wakeup_bitmap_11,
		 wakeup_bitmap_12,
		 wakeup_bitmap_13,
		 wakeup_bitmap_14,
		 wakeup_bitmap_15,
		 wakeup_bitmap_16,
		 wakeup_bitmap_17,
		 wakeup_bitmap_18,
		 wakeup_bitmap_19,
		 wakeup_bitmap_20,
		 wakeup_bitmap_21,
		 wakeup_bitmap_22,
		 wakeup_bitmap_23,
		 wakeup_bitmap_24
	 };


	 for (int i = 0; i < wakeup_bitmap_allArray_LEN; i++) {
        display_bitmap(wakeup_bitmap_allArray[i]);
		
		delay(20);
    }
}





void playLookRightFromMiddleAnimation() {
	// Array of all bitmaps for convenience. (Total bytes used to store images in PROGMEM = 26000)
	const int look_right_from_middle_bitmap_allArray_LEN = 9;
	const unsigned char* look_right_from_middle_bitmap_allArray[9] = {
		look_right_from_middle_bitmap_00,
		look_right_from_middle_bitmap_01,
		look_right_from_middle_bitmap_02,
		look_right_from_middle_bitmap_03,
		look_right_from_middle_bitmap_04,
		look_right_from_middle_bitmap_05,
		look_right_from_middle_bitmap_06,
		look_right_from_middle_bitmap_07,
		look_right_from_middle_bitmap_08,
	};

	for (int i = 0; i < look_right_from_middle_bitmap_allArray_LEN; i++) {
	   display_bitmap(look_right_from_middle_bitmap_allArray[i]);
	   delay(20);
   }
}


void playLookMiddleFromRightAnimation() {
	// Array of all bitmaps for convenience. (Total bytes used to store images in PROGMEM = 26000)
	const int look_middle_from_right_bitmap_allArray_LEN = 8;
	const unsigned char* look_middle_from_right_bitmap_allArray[8] = {
		look_middle_from_right_bitmap_00,
		look_middle_from_right_bitmap_01,
		look_middle_from_right_bitmap_02,
		look_middle_from_right_bitmap_03,
		look_middle_from_right_bitmap_04,
		look_middle_from_right_bitmap_05,
		look_middle_from_right_bitmap_06,
		look_middle_from_right_bitmap_07,
	};

	for (int i = 0; i < look_middle_from_right_bitmap_allArray_LEN; i++) {
	   display_bitmap(look_middle_from_right_bitmap_allArray[i]);
	   delay(20);
   }
}

void playLookLeftFromMiddleAnimation() {
	// Array of all bitmaps for convenience. (Total bytes used to store images in PROGMEM = 26000)
	const int look_left_from_middle_bitmap_allArray_LEN = 9;
	const unsigned char* look_left_from_middle_bitmap_allArray[9] = {
		look_left_from_middle_bitmap_00,
		look_left_from_middle_bitmap_01,
		look_left_from_middle_bitmap_02,
		look_left_from_middle_bitmap_03,
		look_left_from_middle_bitmap_04,
		look_left_from_middle_bitmap_05,
		look_left_from_middle_bitmap_06,
		look_left_from_middle_bitmap_07,
		look_left_from_middle_bitmap_08,
	};

	for (int i = 0; i < look_left_from_middle_bitmap_allArray_LEN; i++) {
	   display_bitmap(look_left_from_middle_bitmap_allArray[i]);
	   delay(20);
   }
}





void playLookMiddleFromLeftAnimation() {
	// Array of all bitmaps for convenience. (Total bytes used to store images in PROGMEM = 26000)
	const int look_middle_from_left_bitmap_allArray_LEN = 8;
	const unsigned char* look_middle_from_left_bitmap_allArray[8] = {
		look_middle_from_left_bitmap_00,
		look_middle_from_left_bitmap_01,
		look_middle_from_left_bitmap_02,
		look_middle_from_left_bitmap_03,
		look_middle_from_left_bitmap_04,
		look_middle_from_left_bitmap_05,
		look_middle_from_left_bitmap_06,
		look_middle_from_left_bitmap_07
	};

	for (int i = 0; i < look_middle_from_left_bitmap_allArray_LEN; i++) {
	   display_bitmap(look_middle_from_left_bitmap_allArray[i]);
	   delay(20);
   }
}




void playNormalToFunnyEyesAnimation() {
	// Array of all bitmaps for convenience. (Total bytes used to store images in PROGMEM = 26000)
	const int funny_eyes_bitmap_allArray_LEN = 6;
	const unsigned char* funny_eyes_bitmap_allArray[6] = {
		funny_eyes_bitmap_00,
		funny_eyes_bitmap_01,
		funny_eyes_bitmap_02,
		funny_eyes_bitmap_03,
		funny_eyes_bitmap_04,
		funny_eyes_bitmap_05,
	};

	for (int i = 0; i < funny_eyes_bitmap_allArray_LEN; i++) {
	   display_bitmap(funny_eyes_bitmap_allArray[i]);
	   delay(20);
   }
}

void playFunnyEyesToNormalAnimation() {
	// Array of all bitmaps for convenience. (Total bytes used to store images in PROGMEM = 26000)
	const int funny_eyes_bitmap_allArray_LEN = 6;
	const unsigned char* funny_eyes_bitmap_allArray[6] = {
		funny_eyes_bitmap_00,
		funny_eyes_bitmap_01,
		funny_eyes_bitmap_02,
		funny_eyes_bitmap_03,
		funny_eyes_bitmap_04,
		funny_eyes_bitmap_05,
	};

	for (int i = funny_eyes_bitmap_allArray_LEN - 1; i >= 0; i--) {
	   display_bitmap(funny_eyes_bitmap_allArray[i]);
	   delay(20);
   }
}




void playTongueOutAnimation() {
	// Array of all bitmaps for convenience. (Total bytes used to store images in PROGMEM = 26000)
	const int tongue_out_bitmap_allArray_LEN = 4;
	const unsigned char* tongue_out_bitmap_allArray[4] = {
		tongue_out_bitmap_00,
		tongue_out_bitmap_01,
		tongue_out_bitmap_02,
		tongue_out_bitmap_03,
	};

	for (int i = 0; i < tongue_out_bitmap_allArray_LEN; i++) {
	   display_bitmap(tongue_out_bitmap_allArray[i]);
	   delay(20);
   }
}


// SAD


const int sad_bitmap_allArray_LEN = 15;
	const unsigned char* sad_bitmap_allArray[15] = {
		sad_bitmap_00,
		sad_bitmap_01,
		sad_bitmap_02,
		sad_bitmap_03,
		sad_bitmap_04,
		sad_bitmap_05,
		sad_bitmap_06,
		sad_bitmap_07,
		sad_bitmap_08,
		sad_bitmap_09,
		sad_bitmap_10,
		sad_bitmap_11,
		sad_bitmap_12,
		sad_bitmap_13,
		sad_bitmap_14,
	};

void playIdleToSadAnimation() {
	for (int i = 0; i < sad_bitmap_allArray_LEN; i++) {
	   display_bitmap(sad_bitmap_allArray[i]);
	   delay(20);
   }
}

void playSadToIdleAnimation() {
	for (int i = sad_bitmap_allArray_LEN - 1; i >= 0; i--) {
	   display_bitmap(sad_bitmap_allArray[i]);
	   delay(20);
   }
}



void playTearAnimation() {
	// Array of all bitmaps for convenience. (Total bytes used to store images in PROGMEM = 26000)
	const int tear_bitmap_allArray_LEN = 6;
	const unsigned char* tear_bitmap_allArray[6] = {
		tear_bitmap_00,
		tear_bitmap_01,
		tear_bitmap_02,
		tear_bitmap_03,
		tear_bitmap_04,
		tear_bitmap_05,
	};

	for (int i = 0; i < tear_bitmap_allArray_LEN; i++) {
	   display_bitmap(tear_bitmap_allArray[i]);
	   delay(10);
   }
}


// Array of all bitmaps for convenience. (Total bytes used to store images in PROGMEM = 8320)
const int cry_bitmap_allArray_LEN = 8;
const unsigned char* cry_bitmap_allArray[8] = {
	cry_bitmap_01,
	cry_bitmap_02,
	cry_bitmap_03,
	cry_bitmap_04,
	cry_bitmap_05,
	cry_bitmap_06,
	cry_bitmap_07,
	cry_bitmap_08
};


void playSadToCryAnimation() {
	for (int i = cry_bitmap_allArray_LEN - 1; i >= 0; i--) {
	   display_bitmap(cry_bitmap_allArray[i]);
	   delay(20);
   }
}

void playCryToSadAnimation() {
	for (int i = cry_bitmap_allArray_LEN - 1; i >= 0; i--) {
	   display_bitmap(cry_bitmap_allArray[i]);
	   delay(20);
   }
}

// Array of all bitmaps for convenience. (Total bytes used to store images in PROGMEM = 16640)
const int crying_bitmap_allArray_LEN = 10;
const unsigned char* crying_bitmap_allArray[10] = {
	crying_bitmap_09,
	crying_bitmap_10,
	crying_bitmap_11,
	crying_bitmap_12,
	crying_bitmap_13,
	crying_bitmap_14,
	crying_bitmap_15,
	crying_bitmap_16,
	crying_bitmap_17,
	crying_bitmap_18,
};


void playCryingAnimation() {
	for (int i = 0; i < crying_bitmap_allArray_LEN; i++) {
	   display_bitmap(crying_bitmap_allArray[i]);
	   delay(30);
   }
}



// Array of all bitmaps for convenience. (Total bytes used to store images in PROGMEM = 14560)
const int angry_bitmap_allArray_LEN = 14;
const unsigned char* angry_bitmap_allArray[14] = {
	angry_bitmap_00,
	angry_bitmap_01,
	angry_bitmap_02,
	angry_bitmap_03,
	angry_bitmap_04,
	angry_bitmap_05,
	angry_bitmap_06,
	angry_bitmap_07,
	angry_bitmap_08,
	angry_bitmap_09,
	angry_bitmap_10,
	angry_bitmap_11,
	angry_bitmap_12,
	angry_bitmap_13
};


void playIdleToAngryAnimation() {
	for (int i = 0; i < angry_bitmap_allArray_LEN; i++) {
	   display_bitmap(angry_bitmap_allArray[i]);
	   delay(20);
   }
}

void playAngryToIdleAnimation() {
	for (int i = angry_bitmap_allArray_LEN - 1; i >= 0; i--) {
	   display_bitmap(angry_bitmap_allArray[i]);
	   delay(20);
   }
}




// Array of all bitmaps for convenience. (Total bytes used to store images in PROGMEM = 13520)
const int happy_bitmap_allArray_LEN = 13;
const unsigned char* happy_bitmap_allArray[13] = {
	happy_bitmap_00,
	happy_bitmap_01,
	happy_bitmap_02,
	happy_bitmap_03,
	happy_bitmap_04,
	happy_bitmap_05,
	happy_bitmap_06,
	happy_bitmap_07,
	happy_bitmap_08,
	happy_bitmap_09,
	happy_bitmap_10,
	happy_bitmap_11,
	happy_bitmap_12
};


void playIdleToHappyAnimation() {
	for (int i = 0; i < happy_bitmap_allArray_LEN; i++) {
	   display_bitmap(happy_bitmap_allArray[i]);
	   delay(20);
   }
}

void playHappyToIdleAnimation() {	
	for (int i = happy_bitmap_allArray_LEN - 1; i >= 0; i--) {
	   display_bitmap(happy_bitmap_allArray[i]);
	   delay(20);
   }
}




// Array of all bitmaps for convenience. (Total bytes used to store images in PROGMEM = 4160)
const int enjoy_start_bitmap_allArray_LEN = 4;
const unsigned char* enjoy_start_bitmap_allArray[4] = {
	enjoy_start_bitmap_00,
	enjoy_start_bitmap_01,
	enjoy_start_bitmap_02,
	enjoy_start_bitmap_03
};


void playEnjoyStartAnimation() {
	for (int i = 0; i < enjoy_start_bitmap_allArray_LEN; i++) {
	   display_bitmap(enjoy_start_bitmap_allArray[i]);
	   delay(20);
   }
}

void playEnjoyEndAnimation() {	
	for (int i = enjoy_start_bitmap_allArray_LEN - 1; i >= 0; i--) {
	   display_bitmap(enjoy_start_bitmap_allArray[i]);
	   delay(20);
   }
}



// Array of all bitmaps for convenience. (Total bytes used to store images in PROGMEM = 8320)
const int enjoy_bitmap_allArray_LEN = 8;
const unsigned char* enjoy_bitmap_allArray[8] = {
	enjoy_bitmap_04,
	enjoy_bitmap_05,
	enjoy_bitmap_06,
	enjoy_bitmap_07,
	enjoy_bitmap_08,
	enjoy_bitmap_09,
	enjoy_bitmap_10,
	enjoy_bitmap_11
};


void playEnjoyingAnimation() {
	for (int i = 0; i < enjoy_bitmap_allArray_LEN; i++) {
	   display_bitmap(enjoy_bitmap_allArray[i]);
	   delay(20);
   }
}


void playTickleStartAnimation() {
	for (int i = 0; i < tickle_start_bitmap_allArray_LEN; i++) {
	   display_bitmap(tickle_start_bitmap_allArray[i]);
	   delay(20);
   }
}

void playTickleEndAnimation() {
	for (int i = tickle_start_bitmap_allArray_LEN - 1; i >= 0; i--) {
	   display_bitmap(tickle_start_bitmap_allArray[i]);
	   delay(20);
   }
}

void playTickleAnimation() {
	// tickle left
	for (int i = 0; i < tickle_left_bitmap_allArray_LEN; i++) {
	   display_bitmap(tickle_left_bitmap_allArray[i]);
	   delay(2);
   }
   // tickle normal (reverse)
   for (int i = tickle_left_bitmap_allArray_LEN - 2; i >= 0; i--) {
	   display_bitmap(tickle_left_bitmap_allArray[i]);
	   delay(2);
   }
   // tickle right
   for (int i = 0; i < tickle_right_bitmap_allArray_LEN; i++) {
	   display_bitmap(tickle_right_bitmap_allArray[i]);
	   delay(2);
   }
   // tickle normal (reverse)
   for (int i = tickle_right_bitmap_allArray_LEN - 1; i >= 0; i--) {
	   display_bitmap(tickle_right_bitmap_allArray[i]);
	   delay(2);
   }
}