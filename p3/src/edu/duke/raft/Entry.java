package edu.duke.raft;

import java.io.IOException;
import java.io.Serializable;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;

public class Entry implements Serializable {

  public int action;
  public int term;

  // @param entry's action
  // @param entry's term
  public Entry (int action, int term) {
    this.action = action;
    this.term = term;
  }

  public Entry (Entry e) {
    action = e.action;
    term = e.term;
  }

  public String toString () {
    return term + " " + action;
  }  
}

  
