import sys, re, praw, webbrowser
def main():
    
    #Check for proper parameters
    if len(sys.argv) < 3:
        print 'Usage: python',sys.argv[0], '\'subreddit\' \'a\' \'team\''
        print 'Example1: python', sys.argv[0], 'soccerstreams Barcelona'
        print 'Example2: python', sys.argv[0], 'NHLStreams Calgary Flames'
        sys.exit()
    if len(sys.argv) == 3:
        connect(sys.argv[1], sys.argv[2])
    else:
        team = sys.argv[2]
        team += ' '
        team += sys.argv[3]
        connect(sys.argv[1], team)  
    

def connect(sub, team):
    post_limit = 25
    p = re.compile(team, re.IGNORECASE)
    
    # Connect to reddit and download the subreddit front page
    r = praw.Reddit(user_agent='Sport Streams v1.0 by /u/a1ibs') 
    submissions = r.get_subreddit(sub).get_hot(limit=post_limit)
  
    #Process all the submissions from the specificed subreddit
    for submission in submissions:
        m = p.search(submission.title)
        if m:
            print sys.argv[0], ": Found it!", submission.title
            print sys.argv[0], ": Working on finding stream"
            get_stream(r, submission.id)
    print "Error: No thread found with that team, please try again"
    sys.exit()        
 
def get_stream(r, title_id):

    #John Gruber's regex to find URLs in plain text, converted to Python/Unicode
    #See: http://daringfireball.net/2010/07/improved_regex_for_matching_urls  
    url = re.compile(ur'(?i)\b((?:https?://|www\d{0,3}[.]|[a-z0-9.\-]+[.][a-z]{2,4}/)(?:[^\s()<>]+|\(([^\s()<>]+|(\([^\s()<>]+\)))*\))+(?:\(([^\s()<>]+|(\([^\s()<>]+\)))*\)|[^\s`!()\[\]{};:\'".,<>?\xab\xbb\u201c\u201d\u2018\u2019]))')
    
    youtube = re.compile('youtu\.?be(?:\.com)?\/?.*(?:watch|embed)?(?:.*v=|v\/|\/)([\w\-_]+)\&?')
    link = 'http://www.'
    submission = r.get_submission(submission_id=title_id)
    
    #Flatten comments
    comments = praw.helpers.flatten_tree(submission.comments)
    
    #Search through the comments until a link is found
    for comment in comments:
    
        #Use regex to find a match for a url or youtube
        m = re.search(url, comment.body)
        m2 = re.search(youtube, comment.body)
        
        #Youtube check first which will instantly load and then quit
        if m2:
            print sys.argv[0], ": Found YouTube link!", m2.group(0)
            print sys.argv[0], ": Loading link"
            link += m2.group(0)
            webbrowser.open(link)
            sys.exit()
            
        #If a url is found ask the user if they would like to load it otherwise keep looking
        if m:
            print sys.argv[0], ": Found one!", m.group(0)
            load = raw_input('Would you like to load this link?(y/n)')
            if load == 'y':
                print sys.argv[0], ": Loading link"
                webbrowser.open(m.group(0))
                sys.exit()
            else:
                print sys.argv[0], ": Ok still looking"
    sys.exit()

if __name__ == "__main__":
    main()
