//暴力
class Solution {
public:
    int findMaxForm(vector<string>& strs, int m, int n) {
        return f(strs,0,m,n);
    }
    int f(vector<string>&strs,int i,int z,int o){
        if(i==strs.size()){
            return 0;
        }
        int p1=f(strs,i+1,z,o);
        int p2=0;
        int zeros=0,ones=0;
        for(int j=0;j<strs[i].size();j++){
            if(strs[i][j]=='1')ones++;
            else zeros++;
        }
        if(ones<=o&&zeros<=z){
            p2=1+f(strs,i+1,z-zeros,o-ones);
        }
        return max(p1,p2);
    }
};

//记忆化搜索
class Solution {
public:
    int dp[605][105][105];
    int findMaxForm(vector<string>& strs, int m, int n) {
        memset(dp,-1,sizeof(dp));
        return f(strs,0,m,n);
    }
    int f(vector<string>&strs,int i,int z,int o){
        if(i==strs.size()){
            return 0;
        }
        if(dp[i][z][o]!=-1){
            return dp[i][z][o];
        }
        int p1=f(strs,i+1,z,o);
        int p2=0;
        int zeros=0,ones=0;
        for(int j=0;j<strs[i].size();j++){
            if(strs[i][j]=='1')ones++;
            else zeros++;
        }
        if(ones<=o&&zeros<=z){
            p2=1+f(strs,i+1,z-zeros,o-ones);
        }
        int ans=max(p1,p2);
        return dp[i][z][o]=ans;
    }
};

//动态规划
class Solution {
public:
    int dp[605][105][105];
    int findMaxForm(vector<string>& s, int m, int n) {
            int len=s.size();
            for(int i=len-1;i>=0;i--){
                int ones=0;
                int zeros=0;
                for(int j=0;j<s[i].size();j++){
                    if(s[i][j]=='1')ones++;
                    else zeros++;
                }
                for(int z=0,p1,p2;z<=m;z++){
                    for(int o=0;o<=n;o++){
                        p1=dp[i+1][z][o];
                        p2=0;
                        if(zeros<=z&&ones<=o){
                            p2=1+dp[i+1][z-zeros][o-ones];
                        }
                        dp[i][z][o]=max(p1,p2);
                    }
                }
            }
            return dp[0][m][n];
    }
    
    
};

//空间压缩
class Solution {
public:
    int findMaxForm(vector<string>& strs, int m, int n) {
        vector<vector<int>>dp(m+1,vector<int>(n+1));
        for(string s:strs){
            int zeros=0;
            int ones=0;
            for(int i=0;i<s.size();i++){
                if(s[i]=='1')ones++;
                else zeros++;
            }
            for(int z=m;z>=zeros;z--){
                for(int o=n;o>=ones;o--){
                    dp[z][o]=max(dp[z][o],1+dp[z-zeros][o-ones]);
                }
            }
        }
        return dp[m][n];
    }
};

