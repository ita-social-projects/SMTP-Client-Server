pipeline {
    agent any
    
    environment {
        REPO_NAME = 'SMTP-Client-Server'
        CHAT_ID = '-501468350'
        TELEGRAM_API_CREDENTIALS_ID = 'telegram-api-token'
    }

    stages {
        stage('Checkout repository') {
            steps {
                dir(env.REPO_NAME) {
                    cleanWs()

                    checkout scm
                }
            }
        }
        stage('Copy 3rd party libraries') {
           steps{
               dir(env.REPO_NAME){
                    bat "mkdir 3rdPartyLibs"

                    bat "copy C:\\CPP594Libs\\3rdPartyLibs\\*.lib 3rdPartyLibs"

                    bat "copy C:\\CPP594Libs\\3rdPartyLibs\\*.pdb 3rdPartyLibs"
               }
           }
        }
        stage('Build') {
            steps{
                dir(env.REPO_NAME) {
                    bat "msbuild SMTPClientServer.sln"
                }
            }
        }
        stage('Static Analysis') {
           steps{
               dir(env.REPO_NAME){
                   bat "PVS-Studio_Cmd.exe -t SMTPClientServer.sln -C filters.pvsconfig -o report.plog --progress"
               }
           }
        }
    }

    post {
        success {
            script {
                sendToTelegram(
                    env.CHAT_ID,
                    env.TELEGRAM_API_CREDENTIALS_ID,
                    "<b>SUCCESS!</b>%0A<i>branch: ${env.GIT_BRANCH}</i>%0A<a href=\"${env.BUILD_URL}\">build ${env.BUILD_NUMBER}</a>"
                )
            }
        }
        always {
            recordIssues(
                enabledForFailure: true,
                tool: msBuild()
            )
            recordIssues(
               enabledForFailure: true,
               sourceCodeEncoding:'UTF-8',
               tool: PVSStudio(pattern: "${env.REPO_NAME}\\report.plog")
            )
        }
        failure {
            script{
                sendToTelegram(
                    env.CHAT_ID,
                    env.TELEGRAM_API_CREDENTIALS_ID,
                    "<b>FAILURE!</b>%0A<i>branch: ${env.GIT_BRANCH}</i>%0A<a href=\"${env.BUILD_URL}\">build ${env.BUILD_NUMBER}</a>"
                )
            }
        }
    }

}

def sendToTelegram(chatId, credentialsId, messageText) {
    withCredentials([string(credentialsId: 'telegram-api-token', variable: 'TOKEN')]){
        sh """
            curl -X POST \
                 -H "Authorization: Bearer $TOKEN" \
                 -d "chat_id=${chatId}" \
                 -d 'message=${messageText}' \
                 -d "parse_mode=HTML" \
                 http://35.188.163.71:5000/sendtotelegram/ >> /dev/null
        """
    }
}