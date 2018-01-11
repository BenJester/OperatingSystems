package edu.duke.raft;

import java.io.IOException;
import java.io.OutputStream;
import java.io.FileOutputStream;
import java.nio.charset.StandardCharsets;
import java.nio.channels.Channels;
import java.nio.file.Files;
import java.nio.file.FileSystems;
import java.nio.file.Path;
import java.nio.file.StandardCopyOption;
import java.nio.file.StandardOpenOption;
import java.util.List;
import java.util.LinkedList;

public class RaftLog {
  private LinkedList<Entry> mEntries;
  private Path mLogPath;
  
  public RaftLog (String file) {
    mEntries = new LinkedList<Entry> ();
    try {
      mLogPath = FileSystems.getDefault ().getPath (file);
      String delims = " ";
      List<String> lines = Files.readAllLines (mLogPath, 
					       StandardCharsets.US_ASCII);
      Entry e = null;
      
      for (String line : lines) {
	String[] tokens = line.split (delims);
	if ((tokens != null) && (tokens.length > 0)) {
	  e = new Entry (Integer.parseInt (tokens[1]),
			 Integer.parseInt (tokens[0]));
	  mEntries.add (e);
	} else {
	  System.out.println ("Error parsing log line: " + line);
	}	
      }      
    } catch (IOException e) {
      System.out.println (e.getMessage ());
      e.printStackTrace();
    }    
  }

  // Blindly append entries to the end of the log. Note that there is
  // no check to make sure that the last entry is from the correct
  // term. This method should only be used in testing. 
  // 
  // @param entries to append (in order of 0 to append.length-1)
  // @return highest index in log after entries have been appended.
  public int append (Entry[] entries) {
    try {
      if (entries != null) {
	OutputStream out = Files.newOutputStream (mLogPath, 
						  StandardOpenOption.CREATE,
						  StandardOpenOption.APPEND,
						  StandardOpenOption.SYNC);
	for (Entry entry : entries) {
	  if (entry != null) {
	    out.write (entry.toString ().getBytes ());
	    out.write ('\n');
	    mEntries.add (entry);
	  } else {
//	    System.out.println ("Tried to append null entry to RaftLog.");
	    break;
	  }	
	}
	out.close ();
      }
    } catch (IOException e) {
      System.out.println (e.getMessage ());
      e.printStackTrace();
    } 
    return (mEntries.size () - 1);
  }
  
  // @param entries to append (in order of 0 to append.length-1). must
  // be non-null.
  // @param index of log entry before entries to append (-1 if
  // inserting at index 0)
  // @param term of log entry before entries to append (ignored if
  // prevIndex is -1)
  // @return highest index in log after entries have been appended, if
  // the entry at prevIndex is not from prevTerm or if the log does
  // not have an entry at prevIndex, the append request will fail, and
  // the method will return -1.
  public int insert (Entry[] entries, int prevIndex, int prevTerm) {
    if (entries == null) {
      // cannot insert null entries
      return -1;
    } else if ((prevIndex == -1) ||
	       ((mEntries.size () > prevIndex) &&
		(mEntries.get (prevIndex) != null) &&
		(mEntries.get (prevIndex).term == prevTerm))) {
      // Because we are inserting in the middle of our log, we
      // will update our log by creating a temporary on-disk log
      // with the new entries and then replacing the old on-disk
      // log with the temporary one.

      // First, create an in-memory copy of the existing log up to
      // the point where the new entries will be added
      LinkedList<Entry> tmpEntries = new LinkedList<Entry> ();
      for (int i=0; i<=prevIndex; i++) {
	Entry entry = mEntries.get (i);
	tmpEntries.add (entry);
      }
      Path tmpLogPath;
      // Next, add the new entries to temporary in-memory and
      // on-disk logs
      try {
	tmpLogPath = 
	  FileSystems.getDefault ().
	  getPath (mLogPath.toAbsolutePath ().toString () + ".tmp");
	
	OutputStream out = 
	  Files.newOutputStream (tmpLogPath, 
				 StandardOpenOption.CREATE,
				 StandardOpenOption.TRUNCATE_EXISTING,
				 StandardOpenOption.SYNC);
	
	// Write out the prefix
	for (Entry entry : tmpEntries) {
	  out.write (entry.toString ().getBytes ());
	  out.write ('\n');
	}
	
	// Add the new entries
	for (Entry entry : entries) {
	  if (entry != null) {
	    out.write (entry.toString ().getBytes ());
	    out.write ('\n');
	    tmpEntries.add (entry);
	  }
	}
	out.close ();
      } catch (IOException e) {
	System.out.println ("Error creating temporary log.");
	System.out.println (e.getMessage ());
	e.printStackTrace();
	return -1;
      }

      // Switch the on-disk log to the new version
      try {
	Files.move (tmpLogPath, 
		    mLogPath, 
		    StandardCopyOption.REPLACE_EXISTING,
		    StandardCopyOption.ATOMIC_MOVE);
      } catch (IOException e) {
	System.out.println ("Error replacing old log.");
	System.out.println (e.getMessage ());
	e.printStackTrace();
	return -1;
      }
      
      // Switch the in-memory log to the new version
      mEntries = tmpEntries;
    } else {
      System.out.println (
	"RaftLog: " +
	"index and term mismatch, could not insert new log entries.");
      return -1;
    }
    
    return (mEntries.size () - 1);
  }

  // @return index of last entry in log
    public int getLastIndex () {
      return (mEntries.size () - 1);
    }

    // @return term of last entry in log
    public int getLastTerm () {
      Entry entry = mEntries.getLast ();
      if (entry != null) {
	return entry.term;
      }
      return -1;
    }

    // @return entry at passed-in index, null if none
    public Entry getEntry (int index) {
      if ((index > -1) && (index < mEntries.size())) {
	return new Entry (mEntries.get (index));
      }
    
      return null;
    }

    public String toString () {
      String toReturn = "{";
      for (Entry e: mEntries) {
	toReturn += " (" + e + ") ";
      }
      toReturn += "}";
      return toReturn;
    }  

    private void init () {
    }

    public static void main (String[] args) {
      if (args.length != 1) {
	System.out.println("usage: java edu.duke.raft.RaftLog <filename>");
	System.exit(1);
      }
      String filename = args[0];
      RaftLog log = new RaftLog (filename);
      System.out.println ("Initial RaftLog: " + log);

      Entry[] entries = new Entry[1];
      entries[0] = new Entry (0, 0);
      System.out.println("Appending new entry " + entries[0] + ".");
      log.append (entries);
      System.out.println ("Resulting RaftLog: " + log);

      Entry firstEntry = log.getEntry (0);
      Entry newEntry = new Entry (1, 3);

      System.out.println("Inserting entry " + newEntry + " at index 1.");
      entries[0] = newEntry;
      log.insert (entries, 0, firstEntry.term);
      System.out.println ("Resulting RaftLog: " + log);

      newEntry.term = 5;
      newEntry.action = 5;    
      System.out.println("Inserting entry " + newEntry + " at index 0.");
      entries[0] = newEntry;
      log.insert (entries, -1, -1);
      System.out.println ("Resulting RaftLog: " + log);
    }

  }
