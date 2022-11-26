// Below, TCP-related functions

// Below, UDP-related functions

// Below, miscellaneous functions

unsigned int initialAvailableMistakes(unsigned int wordLength) {
  if (wordLength <= 6) {
    return 7;
  } else if (7 >= wordLength && wordLength <= 10) {
    return 8;
  }
  return 9;
}