package edu.duke.raft;

import java.io.IOException;
import java.io.OutputStream;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.FileSystems;
import java.nio.file.Path;
import java.nio.file.StandardOpenOption;
import java.util.List;

public class RaftConfig {

  private int mCurrentTerm=0;
  private int mVotedFor=0;
  private int mNumServers=0;
  private int mTimeoutOverride=0;
  private Path mConfigPath=null;
  
  final private String CURRENT_TERM = "CURRENT_TERM";
  final private String VOTED_FOR = "VOTED_FOR";
  final private String NUM_SERVERS = "NUM_SERVERS";
  final private String ELECTION_TIMEOUT_OVERRIDE = "ELECTION_TIMEOUT_OVERRIDE";

  // @param file where config log is stored
  public RaftConfig (String file) {

    try {
      mTimeoutOverride = -1;
      mConfigPath = FileSystems.getDefault().getPath (file);
      String delims = "=";
      List<String> lines = Files.readAllLines (mConfigPath, 
					       StandardCharsets.US_ASCII);
      int lineNum=1;
      for (String line : lines) {
	String[] tokens = line.split (delims);
	if ((tokens != null) && (tokens.length == 2)) {
	  String field = tokens[0];
	  String value = tokens[1];
	  if (field.equals (CURRENT_TERM)) {
	    mCurrentTerm = Integer.parseInt (value);
	  } else if (field.equals (VOTED_FOR)) {
	    mVotedFor = Integer.parseInt (value);
	  } else if (field.equals (NUM_SERVERS)) {
	    mNumServers = Integer.parseInt (value);
	  } else if (field.equals (ELECTION_TIMEOUT_OVERRIDE)) {
	    mTimeoutOverride = Integer.parseInt (value);
	  } else {
	  System.out.println ("Error parsing " + 
			      file + 
			      "." + 
			      lineNum + 
			      ": " + 
			      field);
	  }
	} else {
	  System.out.println ("Error parsing " + 
			      file + 
			      "." + 
			      lineNum + 
			      ": " + 
			      line);
	}	
	lineNum++;
      }      
    } catch (IOException e) {
      System.out.println (e.getMessage ());
    }    
  }

  // @param new term. if new term is larger than current term it will
  // be synchronously written term to the config log. otherwise the
  // current term will remain the same.
  // @param server voted for in the current term (0 if none).
  public void setCurrentTerm (int term, int votedFor) {
    if (term > mCurrentTerm ) {
      try {
	OutputStream out = Files.newOutputStream (mConfigPath, 
						  StandardOpenOption.APPEND,
						  StandardOpenOption.SYNC);
	out.write (
	  new String (CURRENT_TERM + 
		      "=" + 
		      term +
		      '\n' + 
		      VOTED_FOR + 
		      "=" + 
		      votedFor +
		      '\n').getBytes ()
		   );
	out.close ();
	// voting record safely on disk now
	mCurrentTerm = term;
	mVotedFor = votedFor;
      } catch (IOException e) {
	System.out.println (e.getMessage ());
      }
    }
  }

  // @return the current term
  public int getCurrentTerm () {
    return mCurrentTerm;
  }

  // @return who the server voted for in the current term (0 if none)
  public int getVotedFor () {
    return mVotedFor;
  }    

  // @return the number of server
  public int getNumServers () {
    return mNumServers;
  }

  // @return the election timeout override (-1 if use default values)
  public int getTimeoutOverride () {
    return mTimeoutOverride;
  }

  public String toString () {
    return new String (CURRENT_TERM + 
		      "=" + 
		      mCurrentTerm +
		      ", " + 
		      VOTED_FOR + 
		      "=" + 
		      mVotedFor);
  }

  public static void main (String[] args) {
    if (args.length != 1) {
      System.out.println("usage: java edu.duke.raft.RaftConfig <filename>");
      System.exit(1);
    }
    String filename = args[0];
    RaftConfig config = new RaftConfig (filename);
    System.out.println ("RaftConfig: " + config);

    // this should have no effect
    config.setCurrentTerm (config.getCurrentTerm (), 0);
    System.out.println ("RaftConfig: " + config);

    // this should have no effect
    config.setCurrentTerm (config.getCurrentTerm () + 5, 0);
    System.out.println ("RaftConfig: " + config);
  }  
}

