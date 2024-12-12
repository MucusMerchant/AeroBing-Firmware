#ifndef STATUS_ENUMS_H
#define STATUS_ENUMS_H

// Type definition for Status
typedef enum Status {
  UNINITIALIZED = 0,
  UNAVAILABLE = 1,
  PERMANENTLY_UNAVAILABLE = 2,
  AVAILABLE = 3,
} Status;

// Returns the string representation of a status (instead of numerical values)
inline const char* statusToString(Status s) {

  switch (s) {
    case (UNINITIALIZED):           return "UNINITIALIZED";
    case (UNAVAILABLE):             return "UNAVAILABLE";
    case (PERMANENTLY_UNAVAILABLE): return "PERMANENTLY_UNAVAILABLE";
    case (AVAILABLE):               return "AVAILABLE";
    default:                        return "";
  }
}

#endif
