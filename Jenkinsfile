pipeline {
  agent any
  stages {
    stage('Init') {
      steps {
        echo "Testing..."
      }
    }
    stage('Build') {
      parallel {
        stage('echoing') {
          steps {
            echo "Building..."
          }
          post {
            always {
              success {
                echo "Great work"
              }
              failure {
                echo "Better luck next time"
              }
            }
          }
        }
        stage('what hostname do you have?') {
          steps {
            sh 'hostname'
          }
          post {
            always {
              success {
                echo "Great work"
              }
              failure {
                echo "Better luck next time"
              }
            }
          }
        }
        stage('git version') {
          steps {
            sh '/usr/bin/git --version'
          }
          post {
            always {
              success {
                echo "Great work"
              }
              failure {
                echo "Better luck next time"
              }
            }
          }
        }
        stage('gcc version') {
          steps {
            sh 'gcc --version'
          }
          post {
            always {
              success {
                echo "Great work"
              }
              failure {
                echo "Better luck next time"
              }
            }
          }
        }
        stage('g++ version') {
          steps {
            sh 'g++ --version'
          }
          post {
            always {
              success {
                echo "Great work"
              }
              failure {
                echo "Better luck next time"
              }
            }
          }
        }
      }
    }
    stage('Deploy') {
      steps {
        echo "Code deployed."
      }
    }
  }
}
