package edu.duke.raft;
import java.util.Timer;

public class LeaderMode extends RaftMode {
    Timer timer;
    Entry heartbeatArr[] = new Entry[0];
    int[] followerLogIndex;
    private boolean mIsDead = false;

    public void go() {
        synchronized(mLock) {
            int term = mConfig.getCurrentTerm();
            System.out.println("S" + mID + "." + term + ": switched to leader mode.");
            RaftResponses.clearAppendResponses(term);
            //RaftResponses.setTerm(term);
            // send initial empty heartbeat
            for(int n=1; n<mConfig.getNumServers(); n++) {
                if(n != mID) {
                    remoteAppendEntries(n, term, mID, mLog.getLastIndex(), mLog.getLastTerm(), heartbeatArr, mCommitIndex); 
                }
            }

            followerLogIndex = new int[mConfig.getNumServers()+1];
            for(int n=0; n <= mConfig.getNumServers(); n++) {
                followerLogIndex[n] = mLog.getLastIndex();
            }


            timer = scheduleTimer(HEARTBEAT_INTERVAL, 0);
        }
    }

    // check if cadidate's log and index are up-to-date with self
    private boolean upToDate(int lastLogIndex, int lastLogTerm) {
        if(mLog.getLastTerm() != lastLogTerm) {
            return mLog.getLastTerm() < lastLogTerm;
        } else {
            return mLog.getLastIndex() <= lastLogIndex;
        }
    }

    private void send() {
        for(int i=1; i<=mConfig.getNumServers(); i++) {
            if(i != mID) {
                int diff = mLog.getLastIndex() - followerLogIndex[i];
                Entry[] entries = new Entry[diff];
                for(int j=0; j<diff; j++) {
                    entries[j] = mLog.getEntry(followerLogIndex[i] + j + 1);
                }
                if(mLog.getEntry(followerLogIndex[i]) != null) {
                    remoteAppendEntries(i, mConfig.getCurrentTerm(), mID, followerLogIndex[i], mLog.getEntry(followerLogIndex[i]).term, entries, mCommitIndex);
                } else { // if leader log is empty, mLog.getEntry(followerLogIndex[i]) is null, whose term does not exist, so send current term as term.
                    remoteAppendEntries(i, mConfig.getCurrentTerm(), mID, followerLogIndex[i], mConfig.getCurrentTerm(), entries, mCommitIndex);
                }
            }
        }
    }

    // @param candidate’s term
    // @param candidate requesting vote
    // @param index of candidate’s last log entry
    // @param term of candidate’s last log entry
    // @return 0, if server votes for candidate; otherwise, server's
    // current term
    public int requestVote(int candidateTerm, int candidateID, int lastLogIndex, int lastLogTerm) {
        synchronized(mLock) {
            if (mIsDead) return mConfig.getCurrentTerm();

            int term = mConfig.getCurrentTerm();
            int vote = term;

            if(term >= candidateTerm) {
                return vote; // nay vote
            } else {
                timer.cancel();
                vote = candidateTerm;

                if(upToDate(lastLogIndex, lastLogTerm)) {
                    mConfig.setCurrentTerm(candidateTerm, candidateID);
                    vote = 0;
                    // System.out.println("S" + mID + "." + term + " votes for S" + candidateID + " new term: " + candidateID);
                } else {
                    mConfig.setCurrentTerm(candidateTerm, 0);
                }
                mIsDead = true;
                RaftMode follower = new FollowerMode();
                RaftServerImpl.setMode(follower);
                return vote;
            }
        }
    }

    // @param leader’s term
    // @param current leader
    // @param index of log entry before entries to append
    // @param term of log entry before entries to append
    // @param entries to append (in order of 0 to append.length-1)
    // @param index of highest committed entry
    // @return 0, if server appended entries; otherwise, server's
    // current term
    public int appendEntries(int leaderTerm, int leaderID, int prevLogIndex, int prevLogTerm, Entry[] entries, int leaderCommit) {
        synchronized(mLock) {
            if (mIsDead) return mConfig.getCurrentTerm();

            int term = mConfig.getCurrentTerm();
            int result = term;

            if(term >= leaderTerm) {
                return result;
            } else {
                timer.cancel();
                mConfig.setCurrentTerm(leaderTerm, 0);
                result = leaderTerm;

                if (entries.length == 0) {
                    if (prevLogIndex == mLog.getLastIndex() && prevLogTerm == mLog.getLastTerm()) { 
                        mIsDead = true;
                        RaftMode follower = new FollowerMode();
                        RaftServerImpl.setMode(follower);
                        return 0;

                    } else {
                        mIsDead = true;
                        RaftMode follower = new FollowerMode();
                        RaftServerImpl.setMode(follower);
                        return leaderTerm;

                    }
                }

                if(mLog.insert(entries, prevLogIndex, prevLogTerm) != -1) {
                    result = 0;
                    // System.out.println("S" + mID + "." + term + "receives appendEntry from " + leaderID);
                } 
                mIsDead = true;
                RaftMode follower = new FollowerMode();
                RaftServerImpl.setMode(follower);
                return result;
            }
        }
    }

    // @param id of the timer that timed out
    public void handleTimeout(int timerID) {
        synchronized(mLock) {
            if (mIsDead) return;

            timer.cancel();
            int[] appendResponses = RaftResponses.getAppendResponses(mConfig.getCurrentTerm());

            if(appendResponses != null) { // if receives response, modify followerLogIndex
                for(int i=1; i<appendResponses.length; i++) {
                    if(i != mID) {
                        if(appendResponses[i] != 0 && appendResponses[i] <= mConfig.getCurrentTerm()) {
                            if(followerLogIndex[i] != -1) {
                                followerLogIndex[i] -= 1;
                            }
                        } else if (appendResponses[i] != 0 && appendResponses[i] > mConfig.getCurrentTerm()) {
                            RaftResponses.clearAppendResponses(mConfig.getCurrentTerm());
                            mConfig.setCurrentTerm(appendResponses[i], 0);
                            mIsDead = true;
                            
                            RaftMode follower = new FollowerMode();
                            RaftServerImpl.setMode(follower);
                            return;
                        } else if(appendResponses[i] == 0) { // response is 0, update lastIndex
                            followerLogIndex[i] = mLog.getLastIndex();
                        }
                    }
                }
            }

            /*
            for(int n=1; n<mConfig.getNumServers(); n++) {
                if(n != mID) {
                    remoteAppendEntries(n, term, mID, mLog.getLastIndex(), mLog.getLastTerm(), heartbeatArr, mCommitIndex); 
                }
            }
            */
            RaftResponses.clearAppendResponses(mConfig.getCurrentTerm());
            /*
            for(int n=1; n <= mConfig.getNumServers(); n++) {
                System.out.println("follower log: t" + n + " " + followerLogIndex[n]);
            }
            */
            // send heartbeat
            send();
            timer = scheduleTimer(HEARTBEAT_INTERVAL, 0);
        }
    }   
}
