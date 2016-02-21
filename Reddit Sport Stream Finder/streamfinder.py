import sys, re, praw, webbrowser
def main():
    
    #Check for proper parameters
    if len(sys.argv) < 3:
        print 'Usage: python',sys.argv[0], '\'subreddit\' \'team\''
        sys.exit()
    elif(sys.argv[1] != "soccerstreams"):
        print 'Error: only soccerstreams subreddit is supported'
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
    #use regex to find the proper reddit Game Thread post
    # Connect to reddit and download the subreddit front page
    r = praw.Reddit(user_agent='Sport Streams v1.0 by /u/Qaz3r') 
    submissions = r.get_subreddit(sub).get_hot(limit=post_limit)
  
    #Process all the submissions from the specificed subreddit
    for submission in submissions:
        m = p.search(submission.title)
        if m:
            print "Found it!", submission.title
            print "Working on finding stream"
            get_stream(r, submission.id)
    print "Error: No thread found with that team, please try again"
    sys.exit()        
 
def get_stream(r, title_id):
    url = re.compile('http[s]?://(?:[a-zA-Z]|[0-9]|[$-_@.&+]|[!*\(\),]|(?:%[0-9a-fA-F][0-9a-fA-F]))+')
    youtube = re.compile('youtu\.?be(?:\.com)?\/?.*(?:watch|embed)?(?:.*v=|v\/|\/)([\w\-_]+)\&?')
    link = 'http://www.'
    submission = r.get_submission(submission_id=title_id)
    comments = praw.helpers.flatten_tree(submission.comments)
    for comment in comments:
        m = re.search(url, comment.body)
        m2 = re.search(youtube, comment.body)
        if m2:
            print "Found YouTube link!", m2.group(0)
            print "Loading link"
            link += m2.group(0)
            webbrowser.open(link)
            sys.exit()
        if m:
            print "Found one!", m.group(0)
            load = raw_input('Would you like to load this link?(y/n)')
            if load == 'y':
                print "Loading link"
                webbrowser.open(m.group(0))
                sys.exit()
            else:
                print 'Ok still looking'
    sys.exit()
def parse():
    '''soup = BeautifulSoup(http://imgur.com/a/VqUKy)
    matches = soup.select('.album-view-image-link a')
    matches[0]['href']
    imgurUrlPattern = re.compile(r'(http://i.imgur.com/(.*))(\?.*)?')
    '''
if __name__ == "__main__":
    main()
