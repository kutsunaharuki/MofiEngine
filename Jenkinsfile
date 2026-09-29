pipeline {
    agent any
    triggers {
        pollSCM('H/5 * * * *')
    }
    environment {
        IMAGE    = 'dx12-buildtools:latest'
        SOLUTION = 'Game\\NonExistent.sln'
        PLATFORM = 'x64'
    }
    options {
        timestamps()
        timeout(time: 60, unit: 'MINUTES')
    }
    stages {
        stage('Build') {
            parallel {
                stage('Debug') {
                    steps {
                        bat """
                        docker run --rm -v ${WORKSPACE}:C:\\src ${IMAGE} cmd /c "msbuild C:\\src\\${SOLUTION} /p:Configuration=Debug /p:Platform=${PLATFORM}"
                        """
                    }
                }
                stage('Release') {
                    steps {
                        bat """
                        docker run --rm -v ${WORKSPACE}:C:\\src ${IMAGE} cmd /c "msbuild C:\\src\\${SOLUTION} /p:Configuration=Release /p:Platform=${PLATFORM}"
                        """
                    }
                }
            }
        }
    }
    post {
        success {
            archiveArtifacts artifacts: 'Game/x64/**/*.exe, Game/x64/**/*.pdb',
                              allowEmptyArchive: true,
                              fingerprint: true
        }
        failure {
            script {
                def title    = "❌ MofiEngine #${env.BUILD_NUMBER} - FAILURE"
                def buildUrl = "${env.BUILD_URL}"

                withCredentials([
                    string(credentialsId: 'teams-webhook-url', variable: 'TEAMS_URL')
                ]) {
                    def teamsPayload = groovy.json.JsonOutput.toJson([
                        type: 'message',
                        attachments: [[
                            contentType: 'application/vnd.microsoft.card.adaptive',
                            content: [
                                '$schema': 'http://adaptivecards.io/schemas/adaptive-card.json',
                                type    : 'AdaptiveCard',
                                version : '1.4',
                                body: [
                                    [type: 'TextBlock', text: title, weight: 'Bolder', size: 'Medium', color: 'Attention'],
                                    [type: 'TextBlock', text: "Job: ${env.JOB_NAME}", wrap: true],
                                    [type: 'TextBlock', text: "Build: #${env.BUILD_NUMBER}", wrap: true]
                                ],
                                actions: [[
                                    type : 'Action.OpenUrl',
                                    title: 'ビルドログを開く',
                                    url  : buildUrl
                                ]]
                            ]
                        ]]
                    ])
                    writeFile file: 'teams_payload.json', text: teamsPayload
                    bat 'curl -s -H "Content-Type: application/json" -d @teams_payload.json "%TEAMS_URL%"'
                }
            }
        }
    }
}